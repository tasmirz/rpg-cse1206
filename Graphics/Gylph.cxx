#include <cstdlib>
#include <iostream>
#include <vector>

#include "header.hxx"
using namespace Graphics;
Gylph::Gylph(std::vector<char[4]> ascii, std::vector<Color> color,
             Boundary boundary)
    : boundary(boundary) {
  try {
    if (ascii.size() != color.size()) throw "[Gylph] Sizes not equal\n";
  } catch (std::string err) {
    std::cerr << err;
    exit(-1);
  }
  for (int i = 0; i < ascii.size(); i++) {
    pixels.push_back(Pixel(ascii[i], &color[i]));
  }
}