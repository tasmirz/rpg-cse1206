#include <filesystem>
#include <iostream>
#include <sstream>

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
  if (!writable) return false;
  if (!index_set) return false;

  // Build the payload in memory first so we don't truncate the file
  // before knowing we'll actually write something successfully.
  std::stringstream buf(std::ios::binary | std::ios::out | std::ios::in);
  constexpr char magic[4] = {'S', 'C', 'H', '2'};
  buf.write(magic, 4);
  buf.write(&index[0], index.size() + 1);

  uint32_t scalar_count = static_cast<uint32_t>(scheme.size());
  buf.write(reinterpret_cast<char*>(&scalar_count), sizeof(scalar_count));
  for (auto& i : scheme) {
    buf.write(reinterpret_cast<char*>(&i.second), sizeof(int));
    buf.write(&i.first[0], i.first.size() + 1);
  }

  uint32_t array_count = static_cast<uint32_t>(array_scheme.size());
  buf.write(reinterpret_cast<char*>(&array_count), sizeof(array_count));
  for (auto& i : array_scheme) {
    int tag = Schema::array;
    buf.write(reinterpret_cast<char*>(&tag), sizeof(int));
    buf.write(&i.first[0], i.first.size() + 1);
    buf.write(reinterpret_cast<char*>(&i.second.totalField), sizeof(int));
    for (auto& j : i.second.scheme) {
      buf.write(reinterpret_cast<char*>(&j.second), sizeof(int));
      buf.write(&j.first[0], j.first.size() + 1);
    }
  }

  std::fstream file(filename, std::fstream::binary | std::fstream::out |
                                  std::fstream::trunc);
  if (!file.is_open()) return false;
  buf.seekg(0);
  file << buf.rdbuf();
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
  if (!exists()) {
    writable = true;
    return save();
  }
  file.open(filename, std::fstream::binary | std::fstream::in);
  if (!file.is_open()) return false;

  // Peek at a 4-byte magic so we can detect the legacy on-disk format
  // and translate it to the new layout. Two different epoch files can
  // co-exist on disk across upgrades.
  char magic[4] = {0, 0, 0, 0};
  file.read(magic, 4);
  bool legacy = !(magic[0] == 'S' && magic[1] == 'C' && magic[2] == 'H' &&
                  magic[3] == '2');

  if (legacy) {
    // The 4 bytes we just read were actually the start of the index name.
    // Reconstruct them into index: any non-zero prefix up to the first NUL.
    file.clear();
    file.seekg(0, std::ios::beg);
    char c;
    index.clear();
    while (file.read(&c, sizeof(char)) && c != '\0') index += c;
    if (index.empty()) return false;

    // Legacy format alternates (type int) -> (NUL-terminated name), two
    // records per loop iteration. Read pairs until EOF.
    while (true) {
      int type = 0;
      if (!file.read(reinterpret_cast<char*>(&type), sizeof(int))) break;
      std::string name;
      while (file.read(&c, sizeof(char)) && c != '\0') name += c;
      if (name.empty()) break;
      addField(name, type);

      int second_type = 0;
      if (!file.read(reinterpret_cast<char*>(&second_type), sizeof(int))) break;
      if (second_type == Schema::array) {
        std::string arr_name;
        while (file.read(&c, sizeof(char)) && c != '\0') arr_name += c;
        int fields = 0;
        file.read(reinterpret_cast<char*>(&fields), sizeof(int));
        ArraySchema arr;
        for (int k = 0; k < fields; k++) {
          int atype = 0;
          file.read(reinterpret_cast<char*>(&atype), sizeof(int));
          std::string si;
          while (file.read(&c, sizeof(char)) && c != '\0') si += c;
          if (!si.empty()) arr.addField(si, atype);
        }
        addArrayFields(arr_name, arr);
      } else {
        std::string name2;
        while (file.read(&c, sizeof(char)) && c != '\0') name2 += c;
        if (!name2.empty()) addField(name2, second_type);
      }
    }
    file.close();
    // Re-write in the new format so the next load is unambiguous, but
    // only if this schema is allowed to persist (writable). Otherwise
    // save() would truncate the file before bailing out.
    if (writable) save();
    return true;
  }

  char c;
  index.clear();
  while (file.read(&c, sizeof(char)) && c != '\0') {
    index += c;
  }
  if (index.empty() && !file.eof()) {
    return false;
  }

  uint32_t scalar_count = 0;
  file.read(reinterpret_cast<char*>(&scalar_count), sizeof(scalar_count));
  for (uint32_t i = 0; i < scalar_count && !file.eof(); i++) {
    int type = 0;
    file.read(reinterpret_cast<char*>(&type), sizeof(int));
    std::string s;
    while (file.read(&c, sizeof(char)) && c != '\0') s += c;
    if (!s.empty()) addField(s, type);
  }

  uint32_t array_count = 0;
  file.read(reinterpret_cast<char*>(&array_count), sizeof(array_count));
  for (uint32_t i = 0; i < array_count && !file.eof(); i++) {
    int tag = 0;
    file.read(reinterpret_cast<char*>(&tag), sizeof(int));
    if (tag != Schema::array) break;
    std::string s;
    while (file.read(&c, sizeof(char)) && c != '\0') s += c;
    int fields = 0;
    file.read(reinterpret_cast<char*>(&fields), sizeof(int));
    ArraySchema arr;
    for (int k = 0; k < fields; k++) {
      int atype = 0;
      file.read(reinterpret_cast<char*>(&atype), sizeof(int));
      std::string si;
      while (file.read(&c, sizeof(char)) && c != '\0') si += c;
      if (!si.empty()) arr.addField(si, atype);
    }
    addArrayFields(s, arr);
  }
  file.close();
  return true;
}
