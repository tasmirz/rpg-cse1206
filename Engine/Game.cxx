#include <cstdlib>
#include <iostream>
#include <random>

#include "header.hxx"
using namespace Engine;
Game::Game(int columns, int rows, int level)
    : rows(rows), columns(columns), level(level) {
  grid.set(columns, rows);
  current_food = new RegularFood;

  score = 0;
  mp = new int[rows * columns];
  for (auto i = 0; i < rows * columns; i++) {
    mp[i] = 0;
  }
  snake.fragments.push_back({columns / 2, rows / 2});
  // std::cout << snake.fragments.front().x << std::endl;
  //   snake.fragments.push_back({5, 6});
}
int& Game::operator()(int i, int j) { return *(mp + j * columns + i); }
bool Game::move(DIRECTIONS direction, bool ate) {
  Point head = snake.fragments.front();
  Point update;
  switch (direction) {
    case up:
      update = Point(0, -1);
      break;
    case down:
      update = Point(0, 1);
      break;
    case left:
      update = Point(-1, 0);
      break;
    case right:
      update = Point(1, 0);
      break;
    default:
      return false;
      break;
  }
  Point arb = head + update;

  grid.clean();
  for (auto i = 0; i < rows * columns; i++) {
    mp[i] = 0;
  }
  for (auto it = snake.fragments.begin(); it != snake.fragments.end(); ++it) {
    (*this)(it->x, it->y) = 1;
    grid(it->x, it->y) = (Graphics::Pixel*)a;
  }
  // Wrap into bounds first so the self-collision check below uses valid indices.
  arb.x = (columns + arb.x) % columns;
  arb.y = (rows + arb.y) % rows;
  // Self-collision: if the next head position is occupied by the body,
  // the move is fatal. Ignore the tail since it will move out of the way
  // unless the snake just ate (tail is preserved in that case).
  if (score > 4) {
    Point tail = snake.fragments.back();
    if (arb == tail && !ate) {
      // safe: tail vacates this cell this tick
    } else if ((*this)(arb.x, arb.y) == 1) {
      return false;
    }
  }
  snake.fragments.push_front(arb);
  if (!ate) snake.fragments.pop_back();
  return true;
}
void Game::plot() {
  // No full-screen clear; the grid renders only cells that changed.
  // We still null out cells so the previous frame's pointer is released
  // before the snake body is rewritten this frame.
  for (auto it = snake.fragments.begin(); it != snake.fragments.end(); ++it) {
    grid(it->x, it->y) = (Graphics::Pixel*)a;
  }
}

void Game::display() { grid.displayDiff(); }
void Game::serve() {  // must be called after put
  if (!hasfood) {
    int free_cells = 0;
    for (int i = 0; i < rows * columns; i++) {
      if (mp[i] == 0) free_cells++;
    }
    // If the snake has filled the board the player has won.
    if (free_cells == 0) return;
    // Pick a random empty cell. Bail out (instead of looping forever)
    // if rand keeps landing on the snake body.
    int attempts = 0;
    while (attempts++ < rows * columns * 4) {
      int x = rand() % columns;
      int y = rand() % rows;
      if ((*this)(x, y) == 0) {
        grid(x, y) = (Graphics::Pixel*)B;
        hasfood = true;
        foodLocation = Point(x, y);
        return;
      }
    }
  } else {
    grid(foodLocation.x, foodLocation.y) = (Graphics::Pixel*)B;
  }
}

bool Game::collides() {
  if (!hasfood) return false;
  Point q = snake.fragments.front();
  if (q == foodLocation) {
    score += current_food->give_points();
    return true;
  }
  return false;
}
