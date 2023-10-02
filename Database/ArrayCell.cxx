#include "header.hxx"
DB::Cell& DB::ArrayCell::operator[](std::string key) { return cells[key]; }