#include <list>
/*
Inheritance
Abstract class
polymorphism
operator overloading
files
encapsulation
template
STL
composition agregation
static classes
function overloading ,ovveriding
*/
#include "../Database/header.hxx"
#include "../Graphics/header.hxx"
#ifndef ENGINE
#define ENGINE

using Graphics::Boundary;
using Graphics::Point;
namespace Engine {
static const Graphics::Color *c =
    new Graphics::Color(Graphics::rgb(0, 255, 0, 1),
                        Graphics::rgb(100, 0, 100, 1), Graphics::Color::ITALIC);
static const Graphics::Color *c2 = new Graphics::Color(
    Graphics::rgb(255, 255, 0, 1), Graphics::rgb(100, 0, 0, 1),
    Graphics::Color::ITALIC | Graphics::Color::BLINK);
static const Graphics::Pixel *a =
    new Graphics::Pixel("▒", (Graphics::Color *)c);
static const Graphics::Pixel *B =
    new Graphics::Pixel("▒", (Graphics::Color *)c2);
class Game;
class Food {
 protected:
  Boundary boundary;

 public:
  virtual int give_points() = 0;
};
class RegularFood : public Food {
 public:
  int give_points();
};
class SpecialFood : public Food {
  int give_points();
};
enum DIRECTIONS { up = 0, down = 1, left = 2, right = 3, prev = 4 };
class Snake {
  friend class Game;

 public:
  std::list<Point> fragments;
};
class Game {
 protected:
  Point foodLocation;
  Food *current_food;
  Graphics::Grid grid;
  int *mp;
  int level, rows, columns;
  DIRECTIONS curr = right;
  Snake snake;
  RegularFood rf;
  SpecialFood sf;

 public:
  int score = 0;
  Game(int columns, int rows, int level);
  bool move(DIRECTIONS direction = prev, bool ate = false);
  int &operator()(int i, int j);
  void plot();
  void serve();
  bool hasfood = false;
  void display();
  bool collides();
  bool death_collides();
};

/*
template
Operator overloading
RTTI
*/
};  // namespace Engine

#endif