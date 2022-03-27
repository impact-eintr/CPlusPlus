#ifndef ENTRY_H_
#define ENTRY_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "serialize.h"

#define PUT 0
#define DEL 1

using namespace std;

extern const uint16_t entryHeaderSize; // keySize valueSize mark
extern const uint16_t extraHeaderSize; // totalSize

class entry : Serializable {
public:
  entry() : totalSize(0){};
  entry(std::string key, std::string value, uint16_t mark)
    : key(key), value(value), mark(mark),
      totalSize(key.size() + value.size() + entryHeaderSize) {}
  ~entry() {}

  std::string serialize() {
    OutStream os;
    os << totalSize << mark << key << value;
    return os.str();
  }

  int deserialize(std::string &str) {
    InStream is(str);
    is >> mark >> key >> value;
    return is.size();
  }

  void decodeSize(std::string &str) {
    InStream is(str);
    is >> totalSize;
  }

  uint32_t getSize() const { return totalSize; }

  void print_entry() const {
    std::cout << "key: " + key << " value: " << value << " mark: " << mark
              << std::endl;
  }

public:
  std::string key;
  std::string value;
  uint16_t mark;

private:
  uint32_t totalSize; // 我们期望key是远远小于value的
};

#endif // ENTRY_H_
