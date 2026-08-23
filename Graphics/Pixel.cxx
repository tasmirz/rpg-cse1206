#include <cstring>
#include <iostream>

#include "header.hxx"
using namespace Graphics;

Pixel::Pixel() { color = &Graphics::default_color; }
Pixel::Pixel(const char cell[8], Color* color) : color(color) {
  std::strncpy(this->cell, cell, sizeof(this->cell) - 1);
  this->cell[sizeof(this->cell) - 1] = '\0';
}
std::ostream& operator<<(std::ostream& out, Pixel pixel) {
  out << pixel.color->put() << pixel.cell << Color::reset();
  return out;
}
Pixel& Pixel::setCell(const char c[8]) {
  std::strncpy(cell, c, sizeof(cell) - 1);
  cell[sizeof(cell) - 1] = '\0';
  return *this;
}
Pixel& Pixel::setColor(Color* color) {
  this->color = color;
  return *this;
}
