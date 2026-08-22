#include "header.hxx"
using namespace DB;
#include <iostream>
// call with address of stack

Cell::Cell() : size(0), owns(false), data(nullptr) {}
Cell::~Cell() {
  // We don't free `data` here because some callers (e.g. Row::set) hand
  // us a pointer to user-owned storage, and we don't know who owns it.
  // Heap-allocated pointers are tracked in Row's destructor instead.
}
bool Cell::set(int size, void* data) {
  this->size = size;
  this->owns = false;
  this->data = data;
  return true;
}

bool Cell::setOwned(int size, void* data) {
  this->size = size;
  this->owns = true;
  this->data = data;
  return true;
}

void* Cell::get() { return data; }