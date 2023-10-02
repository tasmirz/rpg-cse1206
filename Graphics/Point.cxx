#include "header.hxx"

using namespace Graphics;
Point::Point() {}
Point::Point(int x, int y) : x(x), y(y) {}
Point Point::operator+(Point q) {
  Point qq(x, y);
  qq.x += q.x;
  qq.y += q.y;
  return qq;
}
bool Point::operator==(Point q) { return (q.x == x && q.y == y); }