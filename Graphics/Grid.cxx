#include <iostream>

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
  prev = new Pixel*[rows * columns + 1111];
  for (int i = 0; i < rows * columns + 1110; i++) {
    graph[i] = (Pixel*)blank_pixel;
    prev[i] = nullptr;
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
    prev = new Pixel*[rows * columns + 10];
    for (int i = 0; i < rows * columns; i++) {
      graph[i] = (Pixel*)blank_pixel;
      prev[i] = nullptr;
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

void Grid::displayDiff() {
  // Move cursor home first, then update only the cells whose pixel pointer
  // changed since the previous frame. Each pixel renders as two terminal
  // columns, so logical column j maps to terminal column (j*2 + 1).
  // ANSI rows/cols are 1-based; add 1 to account for the leading blank
  // line we always print for breathing room.
  for (unsigned i = 0; i < rows; i++) {
    for (unsigned j = 0; j < columns; j++) {
      Pixel* cur = (j < columns && i < rows) ? graph[i * columns + j] : 0;
      if (cur == nullptr) cur = (Pixel*)blank_pixel;
      Pixel* old = prev[i * columns + j];
      if (cur == old) continue;  // unchanged, skip
      // Position cursor: row i+2 (1 extra for the leading newline we print,
      // 1 more because ANSI is 1-based), col j*2+1.
      std::cout << "\033[" << (i + 2) << ";" << (j * 2 + 1) << "H"
                << *cur << *cur;
      prev[i * columns + j] = cur;
    }
  }
  // Park the cursor below the grid so the score line stays visible
  // without flickering the playfield.
  std::cout << "\033[" << (rows + 3) << ";1H" << std::flush;
}

void Grid::clean() {
  for (unsigned i = 0; i < rows * columns; i++) graph[i] = 0;
  // std::cout << rows << " " << columns;
}