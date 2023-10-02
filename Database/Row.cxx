#include <cstring>
#include <fstream>
#include <iostream>

#include "header.hxx"
using namespace DB;

Row::Row(Schema* scheme, std::string location)
    : scheme(scheme), location(location) {}
Row::~Row() {
  for (auto& i : cells) {
    delete &(i.second.data);
  }
  for (auto& i : array_cells) {
    for (auto& j : i.second) {
      for (auto& k : j.cells) {
        delete &(k.second.data);
      }
    }
  }
  // deallocate all cells pointer
}
bool Row::load() {
  this->location = location;
  std::fstream file(location, std::fstream::in | std::fstream::binary);
  for (auto i : scheme->scheme) {
    std::cout << "L" << i.first << std::endl;
    if (i.first.length() <= 1) continue;
    switch (i.second) {
      case Schema::integer: {
        std::cout << "Very Big Reach" << std::endl;
        cells[i.first].data = (void*)new int;
        int kk = 0;
        file.read((char*)cells[i.first].data, sizeof(int));
        std::cout << *((int*)cells[i.first].data) << std::endl;
      } break;
      case Schema::text: {
        char c;
        std::string s = "";
        do {
          file.read(&c, sizeof(char));
          s += c;
        } while (c != 0 && !file.eof());
        char* qqq = new char[s.size() + 1];
        strcpy(qqq, s.c_str());

        cells[i.first].set(Schema::text, qqq);
      } break;
    }
  }
  for (auto i : scheme->array_scheme) {
    if (i.first == "") continue;
    int totalField = 0;
    file.read((char*)&totalField, sizeof(int));
    for (int k = 0; k < totalField; k++) {
      ArrayCell qq;
      for (auto j : i.second.scheme) {
        switch (j.second) {
          case Schema::text: {
            std::string s = "";
            char c;
            while (!file.eof()) {
              file.read(&c, sizeof(char));
              s += c;
              if (!c) break;
            }
            char* qqq = new char[s.size() + 1];
            strcpy(qqq, s.c_str());
            Cell cc;
            cc.set(j.second, qqq);
            qq[j.first] = cc;
          } break;
          case Schema::integer: {
            // needs correction
            int* data = (int*)cells[j.first].data;
            file.read((char*)data, sizeof(int));
          } break;
          case Schema::floating: {
            // needs correction
            double* data = (double*)cells[j.first].data;
            file.read((char*)data, sizeof(double));
          } break;
        }
      }
      array_cells[i.first].push_back(qq);
    }
  }
  file.close();
  loaded = true;
  return true;
};
Cell& Row::operator[](std::string fieldname) { return cells[fieldname]; }
std::vector<ArrayCell>& Row::operator()(std::string fieldname) {
  // std::cout << "reach" << std::endl;
  return array_cells[fieldname];
}

bool Row::save() {
  std::cout << location << std::endl;
  std::cout << index << std::endl;
  std::fstream file(
      location, std::fstream::binary | std::fstream::out | std::fstream::trunc);

  std::cout << location << std::endl;
  for (auto i : scheme->scheme) {
    std::cout << i.first << std::endl;
    if (i.first == "") continue;
    switch (i.second) {
      case Schema::text: {
        char* data = (char*)cells[i.first].data;
        while (1) {
          if (data == 0) break;
          file.write((char*)data, sizeof(char));
          if (!*data) break;
          data++;
        }
      } break;
      case Schema::integer: {
        int* data = (int*)cells[i.first].data;
        if (data != 0) file.write((char*)data, sizeof(int));
      } break;
      case Schema::floating: {
        double* data = (double*)cells[i.first].data;
        file.write((char*)data, sizeof(double));
      }
    }
  }
  for (auto i : scheme->array_scheme) {
    if (i.first == "") continue;
    int to = array_cells[i.first].size();
    std::cout << to << std::endl;
    file.write((char*)&(to), sizeof(int));
    for (int k = 0; k < to; k++) {
      for (auto j : i.second.scheme) {
        switch (j.second) {
          case Schema::text: {
            char* data = (char*)array_cells[i.first][k][j.first].data;
            while (1) {
              file.write((char*)data, sizeof(char));
              if (data == 0) break;
              if (!*data) break;
              data++;
            }
          } break;
          case Schema::integer: {
            int* data = (int*)cells[j.first].data;
            file.write((char*)data, sizeof(int));
          } break;
          case Schema::floating: {
            double* data = (double*)cells[j.first].data;
            file.write((char*)data, sizeof(double));
          } break;
        }
      }
    }
  }
  file.close();
  return true;
}

/* What is a row?
 * Row opens a data
 */
