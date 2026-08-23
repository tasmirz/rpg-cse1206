#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include "Object.hxx"
#include "Player.hxx"
#include "Renderer.hxx"
#include "Tile.hxx"
#include "World.hxx"

namespace {

// Put the terminal into raw, non-blocking mode with VMIN=0 / VTIME=0 so
// read() returns whatever is available. Returns the saved termios so the
// caller can restore it on exit. Original bug: the previous version
// toggled cooked mode on every keypress, so select() only saw a byte
// after the user pressed Enter (canonical-mode line buffering).
termios enterRawMode() {
  termios oldt{};
  tcgetattr(STDIN_FILENO, &oldt);
  termios newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  return oldt;
}

void clearScreen() {
  std::cout << "\033[2J\033[H" << std::flush;
}

}  // namespace

int main() {
  using namespace rpg;

// Spiral outward from (0,0) looking for a walkable, unblocked tile. The
// r=0 case is handled separately because a "ring of radius 0" is just
// the single cell (0,0); the ring test `abs(dx) == r || abs(dy) == r`
// would skip it.
auto findSpawn = [](const World& world, int& sx, int& sy) {
  auto tryCell = [&](const World& w, int x, int y) -> bool {
    if (!tileWalkable(w.tileAt(x, y))) return false;
    if (objectBlocks(w.objectAt(x, y))) return false;
    sx = x;
    sy = y;
    return true;
  };
  if (tryCell(world, 0, 0)) return;
  constexpr int kMaxR = 64;
  for (int r = 1; r <= kMaxR; r++) {
    for (int dy = -r; dy <= r; dy++) {
      for (int dx = -r; dx <= r; dx++) {
        if (std::abs(dx) != r && std::abs(dy) != r) continue;
        if (tryCell(world, dx, dy)) return;
      }
    }
  }
};

  // Seed from time so each run feels new but is reproducible within a run.
  uint32_t seed = static_cast<uint32_t>(
      std::chrono::steady_clock::now().time_since_epoch().count() & 0xFFFFFFFFu);

  constexpr int VIEW_W = 60;
  constexpr int VIEW_H = 24;

  World world(seed, /*radius=*/2);
  Player player;
  Renderer renderer(VIEW_W, VIEW_H);

  // Load chunks around (0,0) before searching for a spawn so chunk
  // misses don't read as grass and skip the spawn check.
  world.ensureChunksAround(0, 0);
  findSpawn(world, player.x, player.y);
  world.ensureChunksAround(player.x, player.y);

  termios oldt = enterRawMode();
  clearScreen();
  std::cout << "\033[?25l" << std::flush;

  bool running = true;
  int dx = 0, dy = 0;             // start stationary
  std::string lastMessage;        // line 2 of the HUD
  auto lastMove = std::chrono::steady_clock::now();

  auto interactHere = [&]() {
    ObjectKind obj = world.objectAt(player.x, player.y);
    if (obj == O_NONE) {
      lastMessage = "There's nothing here to pick up.";
      return;
    }
    if (objectPickup(obj)) {
      lastMessage = std::string("You pick up the ") + objectName(obj) + ".";
      world.clearObject(player.x, player.y);
    } else {
      lastMessage = std::string("You can't pick up the ") + objectName(obj) +
                    ".";
    }
  };

  auto interactAdjacent = [&]() {
    // Adjacent pickups are reported but not picked up from a distance.
    static const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (auto& d : dirs) {
      ObjectKind obj = world.objectAt(player.x + d[0], player.y + d[1]);
      if (obj == O_NONE) continue;
      if (obj == O_TREE || obj == O_BUSH || obj == O_ROCK) {
        lastMessage = std::string("A ") + objectName(obj) +
                      " is blocking the way.";
        return;
      }
      lastMessage = std::string("There is a ") + objectName(obj) +
                    " right next to you.";
      return;
    }
    lastMessage = "Nothing interesting nearby.";
  };

  while (running) {
    world.ensureChunksAround(player.x, player.y);
    int updated = renderer.render(world, player);

    std::string s = "pos=(" + std::to_string(player.x) + "," +
                    std::to_string(player.y) + ")  cells=" +
                    std::to_string(updated) +
                    "  [WASD/arrows move, SPACE stop, E interact, Q quit]";
    renderer.status(s);
    renderer.message(lastMessage);

    // Non-blocking poll: wait up to 30ms for a byte of input.
    struct timeval tv{0, 30000};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0) {
      char buf[8] = {0};
      ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf) - 1);
      if (n <= 0) continue;

      if (buf[0] == 0x1b) {
        // Arrow keys arrive as ESC [ A/B/C/D; drain the trailing bytes
        // so they don't leak into the next key.
        char follow[2] = {0, 0};
        struct timeval stv{0, 15000};
        fd_set sfds;
        FD_ZERO(&sfds);
        FD_SET(STDIN_FILENO, &sfds);
        if (select(STDIN_FILENO + 1, &sfds, nullptr, nullptr, &stv) > 0) {
          ::read(STDIN_FILENO, follow, 2);
        }
        if (follow[0] == '[' && (follow[1] == 'A' || follow[1] == 'B' ||
                                  follow[1] == 'C' || follow[1] == 'D')) {
          switch (follow[1]) {
            case 'A': dx = 0; dy = -1; break;
            case 'B': dx = 0; dy = 1;  break;
            case 'C': dx = 1; dy = 0;  break;
            case 'D': dx = -1; dy = 0; break;
          }
        }
        continue;
      }

      char ch = buf[0];
      switch (ch) {
        case 'q':
        case 'Q': running = false; break;
        case 'w': case 'W': dx = 0; dy = -1; break;
        case 's': case 'S': dx = 0; dy = 1;  break;
        case 'a': case 'A': dx = -1; dy = 0; break;
        case 'd': case 'D': dx = 1;  dy = 0; break;
        case ' ':           dx = 0; dy = 0;  break;
        case 'e': case 'E': {
          ObjectKind here = world.objectAt(player.x, player.y);
          if (here != O_NONE && objectPickup(here)) {
            interactHere();
          } else {
            interactAdjacent();
          }
        } break;
        default: break;
      }
    }

    auto now = std::chrono::steady_clock::now();
    if (dx != 0 || dy != 0) {
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMove)
              .count() >= 150) {
        Player::MoveResult r = player.tryMove(dx, dy, world);
        if (r == Player::MoveResult::BlockedTile) {
          lastMessage = "You can't walk into that.";
        } else if (r == Player::MoveResult::BlockedObject) {
          lastMessage = "Something is in the way.";
        }
        // Only reset the timer on successful movement; bumping it on
        // every frame (including blocked ones) would chew the cadence
        // against walls.
        if (r == Player::MoveResult::Moved) lastMove = now;
      }
    }
  }

  // Restore terminal settings and cursor.
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  std::cout << "\033[?25h\033[0m\n" << std::flush;
  return 0;
}
