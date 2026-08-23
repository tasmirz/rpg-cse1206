#include "Renderer.hxx"

#include <algorithm>
#include <iostream>

namespace rpg {

namespace {
// Emit a 2-character ANSI-coloured cell. Foreground, background, and
// glyph strings come from the glyph tables in Tile.hxx/Object.hxx.
void emitColoured(std::ostream& out, const char* fg, const char* bg,
                  const char* ch) {
  out << "\033[" << fg << ';' << bg << 'm' << ch << "\033[0m";
}
}  // namespace

Renderer::Renderer(int viewW, int viewH) : w_(viewW), h_(viewH) {
  // Sentinel that never matches a real tile so first frame draws everything.
  prevTiles_.assign(w_ * h_, static_cast<Tile>(255));
  prevObjects_.assign(w_ * h_, static_cast<ObjectKind>(255));
  prevPlayer_.assign(w_ * h_, false);
}

int Renderer::render(const World& world, const Player& player) {
  // World coordinates of the top-left viewport corner. Bias so the
  // player sits roughly at the centre.
  int ox = player.x - w_ / 2;
  int oy = player.y - h_ / 2;

  int updated = 0;
  for (int sy = 0; sy < h_; sy++) {
    for (int sx = 0; sx < w_; sx++) {
      int wx = ox + sx;
      int wy = oy + sy;

      size_t i = static_cast<size_t>(sy * w_ + sx);
      Tile t = world.tileAt(wx, wy);
      ObjectKind obj = world.objectAt(wx, wy);
      bool here = (wx == player.x && wy == player.y);

      Tile pt = prevTiles_[i];
      ObjectKind po = prevObjects_[i];
      bool ph = prevPlayer_[i];

      // Skip only when nothing changed. Any transition (player arriving
      // or leaving, object pickup, terrain edge) flips at least one of
      // these three values, so this is sufficient to catch them all.
      if (t == pt && obj == po && here == ph) continue;

      std::cout << "\033[" << (sy + 1) << ";" << (sx * 2 + 1) << "H";

      if (here) {
        // Player glyph overrides the tile: '@@' on the tile's bg colour.
        const Glyph& g = glyph(t);
        std::cout << "\033[" << g.bg << ";1m@@\033[0m";
      } else if (obj != O_NONE) {
        const Glyph& tg = glyph(t);
        const ObjectGlyph& og = objectGlyph(obj);
        emitColoured(std::cout, og.fg, tg.bg, og.ch);
      } else {
        const Glyph& g = glyph(t);
        emitColoured(std::cout, g.fg, g.bg, g.ch);
      }

      prevTiles_[i] = t;
      prevObjects_[i] = obj;
      prevPlayer_[i] = here;
      updated++;
    }
  }
  // Park the cursor below the viewport for the status line.
  std::cout << "\033[" << (h_ + 1) << ";1H" << std::flush;
  return updated;
}

void Renderer::status(const std::string& s) {
  std::cout << "\033[" << (h_ + 2) << ";1H\033[0m" << s << std::flush;
}

void Renderer::message(const std::string& s) {
  std::cout << "\033[" << (h_ + 3) << ";1H\033[0m" << s << std::flush;
}

}  // namespace rpg
