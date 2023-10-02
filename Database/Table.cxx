#include <iostream>

#include "header.hxx"
using namespace DB;

namespace fs = std::filesystem;
Table::Table() {}
Row& Table::row(std::string index) {
  Row* ret = new Row(scheme, location + "/" + name + "/" + index);
  ret->index = index;
  return *ret;
}
Row* Table::operator[](std::string index) {
  for (const auto& entry :
       fs::directory_iterator(location + "/" + name + "/")) {
    if (fs::is_regular_file(entry)) {
      std::cout << "File: " << entry.path().filename() << std::endl;
      if (entry.path().filename().string() == index) {
        std::cout << "Big Reach" << std::endl;
        std::cout << location + "/" + name + "/" + index << std::endl;
        Row* ret = new Row(
            scheme,
            location + "/" + name + "/" +
                index);  // Row(*scheme, entry.path().filename().string());
        ret->index = index;
        ret->load();
        return ret;
      }
    }
  }
  return nullptr;
}
Table::~Table() {
  if (loaded) delete scheme;
}
bool Table::load(std::string location) {
  this->location = location;
  // if (scheme == nullptr) //idk if this will cause problem
  scheme = new Schema();
  loaded = true;
  scheme->load(location + "/" + name + "/Schema");
  return true;
}
bool Table::create(std::string location, Schema* schema) {
  this->location = location;
  // this->name = "yay";
  this->scheme = schema;
  if (!std::filesystem::exists(location + "/" + name)) {
    try {
      if (std::filesystem::create_directory(location + "/" + name)) {
        std::cout << "Table created successfully: " << location + "/" + name
                  << std::endl;
      } else {
        std::cerr << "Failed to create table: " << location << std::endl;
      }
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
  if (scheme == 0) scheme = new Schema();
  scheme->load(location + "/" + name + "/Schema");
  return true;
}
bool Table::exists(std::string str) {
  if (fs::exists(location + "/" + name + "/" + str)) return true;

  return false;
}
bool Table::remove(std::string str) {
  if (exists(str)) {
    std::filesystem::remove(location + "/" + name + "/" + str);
  }
  return true;
}
/* What is a table
 * table finds a row in the filesystem
 * it only finds does not open
 * index must be string for now
 */