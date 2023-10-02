#include <ostream>
#include <vector>
#ifndef GRAPHICS
#define GRAPHICS
namespace Graphics {

class Pixel;
};
std::ostream& operator<<(std::ostream& out, Graphics::Pixel pixel);
namespace Graphics {

struct rgb {
  unsigned r : 8 = 0;
  unsigned g : 8 = 0;
  unsigned b : 8 = 0;
  bool set = false;
};

class Color {
  friend class Pixel;
  rgb foreground;
  rgb background;
  char flags;

 public:
  static const unsigned char BOLD = 1 << 0;
  static const unsigned char DIM = 1 << 1;
  static const unsigned char ITALIC = 1 << 2;
  static const unsigned char UNwDERLINE = 1 << 3;
  static const unsigned char BLINK = 1 << 4;
  static const unsigned char RBLINK = 1 << 5;
  static const unsigned char INVERT = 1 << 6;
  Color();
  Color(rgb foreground, rgb background, char flags = 0);
  std::string put() const;
  static std::string reset();
};

static const Color default_color({0, 0, 0, 1}, {200, 200, 200, 1});
class Pixel {
  const Color* color;
  char cell[4] = " ";
  Pixel& setColor(Color* color);
  Pixel& setCell(char c[4]);

 public:
  Pixel();
  Pixel(char cell[4], Color* color);
  friend std::ostream& ::operator<<(std::ostream& out, Pixel pixel);
};

static const Pixel* blank_pixel = new Pixel("  ", (Color*)&default_color);

class Grid {
  Pixel** graph;
  bool _set = false;
  unsigned rows, columns;

 public:
  Pixel*& operator()(unsigned row, unsigned col);
  Grid(int col, int row);
  Grid();
  void set(int rows, int columns);
  void display();
  void clean();
};
void clear(void);
class Point {
 public:
  Point();
  Point(int x, int y);
  int x, y;
  Point operator+(Point q);
  bool operator==(Point q);
};
struct Boundary {
  Point point;
  int width, height;
};
class Gylph {
  std::vector<Pixel> pixels;
  Boundary boundary;

 public:
  Gylph(std::vector<char[4]> ascii, std::vector<Color> color,
        Boundary boundary);
  void plot(Grid& grid) {}
};
};  // namespace Graphics
#endif