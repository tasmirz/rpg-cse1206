#include <iostream>

#include "header.hxx"
using namespace Graphics;

void Graphics::clear(void) {
  std::cout << "\033[H";
  std::cout << "\033[2J";
}