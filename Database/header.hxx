#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#define vstr(a) (void*)(a)
#ifndef DATABASE
#define DATABASE
namespace DB {

class Table;
class ArrayCell;
class Schema;
class ArraySchema {
  int totalField = 0;

 protected:
  std::map<std::string, int> scheme;

 public:
  friend class Row;
  static constexpr const int integer =
      4;  // constexpxr , intit without constructor call
  static constexpr const int floating = 8;
  static constexpr const int text = -1;
  static constexpr const int boolean = 1;
  friend class Schema;
  ArraySchema& addField(std::string fieldname, int type);
  virtual int getTotalFields();
};

class Schema : public ArraySchema {
  bool opened = false;
  bool writable = false;
  bool index_set = false;
  std::string filename;
  std::string index;

  int type = 0;

 private:
  std::map<std::string, ArraySchema> array_scheme;
  bool exists();
  bool save();
  // first : field name, second : data size
  bool load(std::string);

 public:
  Schema();
  friend class Row;
  static constexpr const int create = 1024;
  static constexpr const int open = 1023;
  static constexpr const int array = -2;
  friend class Table;
  ~Schema();
  Schema& setIndex(std::string);
  Schema& addField(std::string fieldname, int type);
  Schema& addArrayFields(std::string fieldname, ArraySchema& array);
};

class Cell {
  int size;
  friend class Row;

 public:
  void* data;
  bool set(int size, void* data);
  void* get();
  Cell();
  ~Cell();
};

class Row {
  Schema* scheme;
  std::fstream file;
  std::string location;
  std::string index;
  std::map<std::string, Cell> cells;
  std::map<std::string, std::vector<ArrayCell>> array_cells;
  bool loaded = false;

 public:
  friend class Table;
  Row(Schema* scheme, std::string location);
  ~Row();
  Cell& operator[](std::string fieldname);
  std::vector<ArrayCell>& operator()(std::string fieldname);
  bool load();
  bool del_row();
  bool del_arr_index();
  Cell setField();
  bool save();
};

class Table {
  Schema* scheme;
  std::string location;
  std::string name;
  bool loaded = false;

 public:
  friend class Database;
  Table();
  ~Table();
  Table(std::string filename);
  bool exists(std::string);
  bool load(std::string location);
  bool create(std::string location, Schema* schema);
  Row& row(std::string index);  // add a row
  Row* operator[](std::string index);

  bool remove(std::string);
  bool create();
  // int destroy();
};

class ArrayCell {
  friend class Row;
  std::map<std::string, Cell> cells;
  friend class Row;

 public:
  Cell& operator[](std::string);
};

class Database {
  std::string name;

 public:
  std::map<std::string, Table*> tables;
  Table& operator[](std::string);
  Database(std::string);
  bool exist(std::string tablename);
  void add(std::string, Schema* scheme);
  int remove(std::string);
  int list();
  Table& create(std::string name, Schema& scheme);
};

// Database will open tables and table schema
//

/*




*/
/*
Creating a database

    writing scheme i.number of fields then usual

        Database db("folder");
automatically load all tables list()->return vector of all tables access all
    tables with db["Table"] find a row db["Table"]["index"]

    saves all dates with

    Storing data in array : the schema should be able to hold an array

                            array_size(4byte) data(read according to schema)

                                schema array

                            storing sting size
    + string

      writing an array schema i.write type;
ii.write number of fields

*/

}  // namespace DB
#endif