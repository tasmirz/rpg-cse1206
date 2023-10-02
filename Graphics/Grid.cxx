

#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#include "header.hxx"

using namespace Graphics;

Pixel*& Grid::operator()(unsigned col, unsigned row) {
  if (row < rows && col < columns)
    return graph[row * columns + col];
  else
    return graph[0];
  // std::cout << " \nr" << col << " " << row << std::endl;
  // std::cout << " \nR " << columns << " " << rows << std::endl;
}

Grid::Grid(int columns, int rows) : rows(rows), columns(columns) {
  graph = new Pixel*[rows * columns + 1111];
  for (int i = 0; i < rows * columns + 1110; i++) {
    graph[i] = (Pixel*)blank_pixel;
  }
  _set = true;
}
Grid::Grid() {}
void Grid::set(int c, int r) {
  rows = r;
  columns = c;
  try {
    if (_set) throw "[Grid] second time init\n";
    graph = new Pixel*[rows * columns + 10];
    for (int i = 0; i < rows * columns; i++) {
      graph[i] = (Pixel*)blank_pixel;
    }
  } catch (std::string err) {
    std::cerr << err;
    exit(-1);
  }
}
void Grid::display() {
  // system("clear");
  std::cout << "\n\r";
  for (unsigned i = 0; i < rows; i++) {
    for (unsigned j = 0; j < columns; j++) {
      ((*this)(j, i) != 0) ? std::cout << *((*this)(j, i)) << *((*this)(j, i))
                           : std::cout << *blank_pixel;
      // std::cout.flush();
    }
    std::cout << "\n\r";
  }
}

void Grid::clean() {
  for (unsigned i = 0; i < rows * columns; i++) graph[i] = 0;
  // std::cout << rows << " " << columns;
}