#include <algorithm>
#include <iostream>
#include <thread>

#include "./Database/header.hxx";
#include "./Engine/header.hxx"
#include "header.hxx"

char now = 0, prev = 'z';
int max_easy = 0;
bool death = false;
DB::Database db("Snake");
DB::Schema scorecard;
DB::Schema state;
char* f = "score";
Engine::Game game(20, 20, 1);
void setup() { death = false; }
void gameLoop() {
  try {
    while (1) {
      bool state = game.collides();
      if (state) game.hasfood = false;
      if (now != prev) {
        switch (now) {
          case 'w':
            if (prev != 's') prev = 'w';
            break;
          case 's':
            if (prev != 'w') prev = 's';
            break;
          case 'd':
            if (prev != 'a') prev = 'd';
            break;
          case 'a':
            if (prev != 'd') prev = 'a';
            break;
          case 'p':
            // pause
            break;
        }
      }
      bool sst;
      switch (prev) {
        case 'w':
          sst = game.move(Engine::up, state);
          break;
        case 's':
          sst = game.move(Engine::down, state);
          break;
        case 'd':
          sst = game.move(Engine::right, state);
          break;
        case 'a':
          sst = game.move(Engine::left, state);
          break;
      }
      if (!sst && game.score > 4) {
        death = true;
        DB::Row& row = db["scorecard"].row("easy");

        row[f].set(DB::Schema::integer, (void*)&game.score);
        // std::cout << row["score"].data;
        row.save();
        std::cout << "Game Over" << std::endl;
        exit(0);
      }
      game.plot();
      game.serve();
      game.display();
      std::cout << game.score << " Max: " << std::max(game.score, max_easy)
                << std::endl;
      usleep(100000);
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
void inputLoop() {
  try {
    while (death == false) {
      now = getch<char>();
      // std::cout << now;
      // std::cout << prev;
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

int main() {
  scorecard.setIndex("scorecard").addField(f, DB::Schema::integer);

  db.add("scorecard", &scorecard);
  // DB::Database q = db;
  DB::Row* aa = db["scorecard"]["easy"];
  int* rt = (int*)((*aa)[f].get());
  std::cout << *rt;
  max_easy = *rt;

  setup();
  Graphics::clear();
  std::cout << "\nWelcome to Snakeland\n\n" << std::endl;
  std::cout << "1. Continue game\n2. New Game\n3. Show Scores\n\nOption: "
            << std::endl;
  int q;
  while (std::cin >> q) {
    // Graphics::clear();
    std::thread gameloop(gameLoop);
    std::thread inputloop(inputLoop);
    switch (q) {
      case 2:
        gameloop.join();
        inputloop.join();
        break;

      default:
        break;
    }
  }
  return 0;
}

/*int main() {
  pthread_t game;
  pthread_t Engine::Game game(20, 20, 1);

  // std::ios_base::sync_with_stdio(false);
  // std::cin.tie(NULL);

  /* Graphics::Grid g(13, 11);

  Graphics::Color *c = new Graphics::Color(
      Graphics::rgb(255, 255, 0, 1), Graphics::rgb(100, 0, 100, 1),
      Graphics::Color::ITALIC | Graphics::Color::BLINK);
  Graphics::Color *c2 =
      new Graphics::Color(Graphics::rgb(0, 255, 0, 1),
                          Graphics::rgb(0, 0, 200, 1),
Graphics::Color::BLINK);
  // g(0, 0).setCell("$").setColor(c);
  // g(2, 2).setCell("▒").setColor(c2);
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 11; j++) {
      char b[4] = {j % 10 + '0'};
      Graphics::Pixel *a = new Graphics::Pixel(b, c);
      g(i, j) = a;
      g.display();
      g.clean();
      usleep(100000);
      Graphics::clear();
      // system("clear");
    }
  } * /
}
*/