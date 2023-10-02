#include <cstring>
#include <iostream>

#include "header.hxx"
using namespace Graphics;

Pixel::Pixel() { color = &Graphics::default_color; }
Pixel::Pixel(char cell[4], Color* color) : color(color) {
  strcpy(this->cell, cell);
}
std::ostream& operator<<(std::ostream& out, Pixel pixel) {
  out << pixel.color->put() << pixel.cell << Color::reset();
  return out;
}
Pixel& Pixel::setCell(char c[4]) {
  // cell = c;
  strcpy(cell, c);
  return *this;
}
Pixel& Pixel::setColor(Color* color) {
  this->color = color;
  return *this;
}
