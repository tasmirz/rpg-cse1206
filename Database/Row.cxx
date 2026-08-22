#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "header.hxx"
using namespace DB;

Row::Row(Schema* scheme, std::string location)
    : scheme(scheme), location(location) {}
Row::~Row() {
  // Free any heap-allocated cells we own (allocated by Row::load()).
  // User-supplied pointers from Cell::set() are NOT freed here.
  for (auto& kv : cells) {
    Cell& c = kv.second;
    if (c.data != nullptr && c.owns) {
      switch (c.size) {
        case Schema::integer: delete static_cast<int*>(c.data); break;
        case Schema::floating: delete static_cast<double*>(c.data); break;
        case Schema::text: delete[] static_cast<char*>(c.data); break;
        default: break;
      }
      c.data = nullptr;
      c.owns = false;
    }
  }
  for (auto& arr : array_cells) {
    for (auto& ac : arr.second) {
      for (auto& kv : ac.cells) {
        Cell& c = kv.second;
        if (c.data != nullptr && c.owns) {
          switch (c.size) {
            case Schema::integer: delete static_cast<int*>(c.data); break;
            case Schema::floating: delete static_cast<double*>(c.data); break;
            case Schema::text: delete[] static_cast<char*>(c.data); break;
            default: break;
          }
          c.data = nullptr;
          c.owns = false;
        }
      }
    }
  }
}

bool Row::load() {
  std::fstream file(location, std::fstream::in | std::fstream::binary);
  if (!file.is_open()) return false;

  // Check for the magic header. Legacy files (raw field values) lack
  // it, so detect and translate.
  char magic[4] = {0, 0, 0, 0};
  file.read(magic, 4);
  bool legacy = !(magic[0] == 'R' && magic[1] == 'O' && magic[2] == 'W' &&
                  magic[3] == '2');

  if (legacy) {
    // The 4 bytes were an int value. Stash it and rewind so the regular
    // path can read the data using the schema.
    file.clear();
    file.seekg(0, std::ios::beg);
    int legacy_int = *reinterpret_cast<int*>(magic);
    // We don't know which field this belongs to in the legacy format.
    // Pick the first integer/floating field we find; for text we'd need
    // a null terminator, which legacy files don't carry, so fall back to
    // the first integer schema field.
    const std::string* target = nullptr;
    int target_type = -1;
    for (auto& f : scheme->scheme) {
      if (f.second == Schema::integer || f.second == Schema::floating ||
          f.second == Schema::boolean) {
        target = &f.first;
        target_type = f.second;
        break;
      }
    }
    if (target != nullptr) {
      if (target_type == Schema::integer) {
        int* v = new int(legacy_int);
        cells[*target].setOwned(Schema::integer, v);
      } else if (target_type == Schema::floating) {
        double* v = new double(static_cast<double>(legacy_int));
        cells[*target].setOwned(Schema::floating, v);
      } else {
        bool* v = new bool(legacy_int != 0);
        cells[*target].setOwned(Schema::boolean, v);
      }
    }
    file.close();
    loaded = true;
    return true;
  }

  uint32_t count = 0;
  file.read(reinterpret_cast<char*>(&count), sizeof(count));
  for (uint32_t i = 0; i < count && !file.eof(); i++) {
    int type = 0;
    file.read(reinterpret_cast<char*>(&type), sizeof(int));
    std::string name;
    char c;
    while (file.read(&c, sizeof(char)) && c != '\0') name += c;
    if (name.empty()) break;
    if (type == Schema::integer) {
      int* v = new int(0);
      file.read(reinterpret_cast<char*>(v), sizeof(int));
      cells[name].setOwned(Schema::integer, v);
    } else if (type == Schema::floating) {
      double* v = new double(0.0);
      file.read(reinterpret_cast<char*>(v), sizeof(double));
      cells[name].setOwned(Schema::floating, v);
    } else if (type == Schema::text) {
      std::string s;
      while (file.read(&c, sizeof(char)) && c != '\0') s += c;
      char* buf = new char[s.size() + 1];
      std::memcpy(buf, s.c_str(), s.size() + 1);
      cells[name].setOwned(Schema::text, buf);
    } else if (type == Schema::boolean) {
      bool* v = new bool(false);
      file.read(reinterpret_cast<char*>(v), sizeof(bool));
      cells[name].setOwned(Schema::boolean, v);
    }
  }

  for (auto& kv : scheme->array_scheme) {
    if (kv.first.empty()) continue;
    int totalField = 0;
    file.read(reinterpret_cast<char*>(&totalField), sizeof(int));
    for (int k = 0; k < totalField && !file.eof(); k++) {
      ArrayCell qq;
      for (auto& j : kv.second.scheme) {
        char c;
        if (j.second == Schema::text) {
          std::string s;
          while (file.read(&c, sizeof(char)) && c != '\0') s += c;
          char* buf = new char[s.size() + 1];
          std::memcpy(buf, s.c_str(), s.size() + 1);
          Cell cc;
          cc.setOwned(Schema::text, buf);
          qq[j.first] = cc;
        } else if (j.second == Schema::integer) {
          int* v = new int(0);
          file.read(reinterpret_cast<char*>(v), sizeof(int));
          Cell cc;
          cc.setOwned(Schema::integer, v);
          qq[j.first] = cc;
        } else if (j.second == Schema::floating) {
          double* v = new double(0.0);
          file.read(reinterpret_cast<char*>(v), sizeof(double));
          Cell cc;
          cc.setOwned(Schema::floating, v);
          qq[j.first] = cc;
        }
      }
      array_cells[kv.first].push_back(qq);
    }
  }
  file.close();
  loaded = true;
  return true;
};

Cell& Row::operator[](std::string fieldname) { return cells[fieldname]; }
std::vector<ArrayCell>& Row::operator()(std::string fieldname) {
  return array_cells[fieldname];
}

bool Row::save() {
  std::stringstream buf(std::ios::binary | std::ios::out | std::ios::in);

  // Magic header so the loader can distinguish new-format rows from the
  // legacy "just an int" format the snake's high-score table used.
  constexpr char magic[4] = {'R', 'O', 'W', '2'};
  buf.write(magic, 4);

  // Persist the schema's scalar fields in deterministic (alphabetical)
  // order so the on-disk format matches what we expect to read back.
  uint32_t count = static_cast<uint32_t>(scheme->scheme.size());
  buf.write(reinterpret_cast<char*>(&count), sizeof(count));

  for (auto& i : scheme->scheme) {
    const std::string& name = i.first;
    if (name.empty()) continue;
    int type = i.second;
    buf.write(reinterpret_cast<char*>(&type), sizeof(int));
    buf.write(name.c_str(), name.size() + 1);  // NUL-terminated

    Cell& c = cells[name];
    if (c.data == nullptr) {
      // No value set for this field: write a zero placeholder so the
      // load-side reader still finds the expected number of bytes.
      if (type == Schema::integer) {
        int zero = 0;
        buf.write(reinterpret_cast<char*>(&zero), sizeof(int));
      } else if (type == Schema::floating) {
        double zero = 0.0;
        buf.write(reinterpret_cast<char*>(&zero), sizeof(double));
      } else if (type == Schema::boolean) {
        char zero = 0;
        buf.write(&zero, sizeof(char));
      } else {
        char nul = '\0';
        buf.write(&nul, sizeof(char));
      }
      continue;
    }

    switch (type) {
      case Schema::text: {
        const char* data = static_cast<const char*>(c.data);
        if (data == nullptr) {
          char nul = '\0';
          buf.write(&nul, sizeof(char));
        } else {
          size_t len = std::strlen(data);
          buf.write(data, len);
          char nul = '\0';
          buf.write(&nul, sizeof(char));
        }
      } break;
      case Schema::integer: {
        int v = *static_cast<int*>(c.data);
        buf.write(reinterpret_cast<char*>(&v), sizeof(int));
      } break;
      case Schema::floating: {
        double v = *static_cast<double*>(c.data);
        buf.write(reinterpret_cast<char*>(&v), sizeof(double));
      } break;
      case Schema::boolean: {
        char v = *static_cast<bool*>(c.data) ? 1 : 0;
        buf.write(&v, sizeof(char));
      } break;
      default: {
        char nul = '\0';
        buf.write(&nul, sizeof(char));
      } break;
    }
  }

  // Array scheme (best-effort).
  for (auto& i : scheme->array_scheme) {
    const std::string& name = i.first;
    if (name.empty()) continue;
    int to = static_cast<int>(array_cells[name].size());
    buf.write(reinterpret_cast<char*>(&to), sizeof(int));
    for (int k = 0; k < to; k++) {
      for (auto& j : i.second.scheme) {
        Cell& c = array_cells[name][k][j.first];
        if (j.second == Schema::text) {
          const char* data = c.data ? static_cast<const char*>(c.data) : "";
          size_t len = data ? std::strlen(data) : 0;
          buf.write(data, len);
          char nul = '\0';
          buf.write(&nul, sizeof(char));
        } else if (j.second == Schema::integer) {
          int v = c.data ? *static_cast<int*>(c.data) : 0;
          buf.write(reinterpret_cast<char*>(&v), sizeof(int));
        } else if (j.second == Schema::floating) {
          double v = c.data ? *static_cast<double*>(c.data) : 0.0;
          buf.write(reinterpret_cast<char*>(&v), sizeof(double));
        }
      }
    }
  }

  // Only truncate the file once we know the in-memory payload is ready,
  // so a failure (or a no-op save) won't leave an empty file behind.
  std::fstream file(location, std::fstream::binary | std::fstream::out |
                                   std::fstream::trunc);
  if (!file.is_open()) return false;
  buf.seekg(0);
  file << buf.rdbuf();
  file.close();
  return true;
}

/* What is a row?
 * Row opens a data
 */