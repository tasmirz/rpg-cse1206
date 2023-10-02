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
  if (score > 4)
    if ((*this)(arb.x, arb.y) == 1) return false;
  arb.x = (columns + arb.x) % columns;
  arb.y = (rows + arb.y) % rows;
  // std::cout << arb.x << " " << arb.y << std::endl;
  snake.fragments.push_front(arb);
  if (!ate) snake.fragments.pop_back();
  return true;
}
void Game::plot() {
  Graphics::clear();
  grid.clean();
  // for (auto i = 0; i < rows * columns; i++) {
  // mp[i] = 0;
  //}
  for (auto it = snake.fragments.begin(); it != snake.fragments.end(); ++it) {
    //(*this)(it->x, it->y) = 1;
    grid(it->x, it->y) = (Graphics::Pixel*)a;
  }
}

void Game::display() { grid.display(); }
void Game::serve() {  // must be called after put
  if (!hasfood)
    while (1) {
      int x = rand() % columns;
      int y = rand() % rows;
      if ((*this)(x, y) == 0) {
        grid(x, y) = (Graphics::Pixel*)B;
        hasfood = true;
        foodLocation = Point(x, y);
        break;
      }
    }
  else
    grid(foodLocation.x, foodLocation.y) = (Graphics::Pixel*)B;
}

bool Game::collides() {
  Point q = snake.fragments.front();
  if (q == foodLocation) {
    score += current_food->give_points();
    return true;
  }
  return false;
}
