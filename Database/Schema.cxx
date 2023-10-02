#include <filesystem>
#include <iostream>

#include "header.hxx"
using namespace DB;

Schema::Schema() { std::cout << "New Schema" << std::endl; }
Schema::~Schema() {
  // save();
  // if (opened) file.close();
}
Schema& Schema::addField(std::string fieldname, int size) {
  scheme[fieldname] = size;
  return *this;
}
Schema& Schema::addArrayFields(std::string fieldname, ArraySchema& array) {
  array_scheme[fieldname] = array;
  return *this;
}
bool Schema::exists() {
  std::cout << filename << std::endl;
  if (std::filesystem::exists(filename)) return true;
  return false;
}
bool Schema::save() {
  std::fstream file(filename, std::fstream::binary | std::fstream::in |
                                  std::fstream::out | std::fstream::trunc);
  if (!file.is_open()) return false;
  if (!writable) return false;
  if (!index_set) return false;
  file.write(&index[0], index.size() + 1);
  for (auto i : scheme) {
    file.write((char*)&i.second, 4);
    file.write(&i.first[0], i.first.size() + 1);
  }
  for (auto i : array_scheme) {
    file.write((char*)&Schema::array, sizeof(int));
    file.write(&i.first[0], i.first.size() + 1);
    file.write((char*)&i.second.totalField, sizeof(int));

    for (auto j : i.second.scheme) {
      file.write((char*)&j.second, 4);
      file.write(&j.first[0], j.first.size() + 1);
    }
  }
  file.close();
  return true;
}
Schema& Schema::setIndex(std::string index) {
  this->index = index;
  index_set = true;
  return *this;
}
bool Schema::load(std::string filename) {
  this->filename = filename;
  std::fstream file;
  std::cout << "Reach" << std::endl;
  if (exists() == false) {
    writable = true;
    save();
  } else {
    file.open(filename, std::fstream::binary | std::fstream::in);
    char c;

    index = "";
    do {
      file.read(&c, sizeof(char));
      if (c == 0) break;
      index += c;
    } while (!file.eof());
    // read index

    while (!file.eof()) {
      int type;
      // reading number
      file.read((char*)&type, sizeof(int));

      std::string s = "";
      do {
        file.read(&c, sizeof(char));
        if (c == 0) break;
        s += c;
      } while (!file.eof());
      addField(s, type);

      file.read((char*)&type, sizeof(int));

      if (type == Schema::array) {
        // read filed name
        std::string s = "";
        do {
          file.read(&c, sizeof(char));
          if (c == 0) break;
          s += c;
        } while (!file.eof());

        int fields;
        file.read((char*)&(fields), sizeof(int));

        ArraySchema arr;
        for (int i = 0; i < fields; i++) {
          int atype;
          file.read((char*)&atype, sizeof(int));
          std::string si = "";

          do {
            file.read(&c, sizeof(char));
            if (c == 0) break;
            si += c;
          } while (!file.eof());

          if (si.size() > 0) arr.addField(si, atype);
        }
        addArrayFields(s, arr);
      } else {
        std::string s = "";
        do {
          file.read(&c, sizeof(char));
          s += c;
          if (c == 0) break;
        } while (!file.eof());
        if (s.size() > 0) addField(s, type);
      }
    }
  }
  return true;
}
