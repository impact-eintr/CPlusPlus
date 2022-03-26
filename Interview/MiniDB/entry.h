#ifndef ENTRY_H_
#define ENTRY_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>

#include "serialize.h"

using namespace std;

extern const uint16_t entryHeaderSize; // keySize valueSize mark
extern const uint16_t extraHeaderSize; // totalSize

class entry : Serializable
{
public:
  entry() = default;
  entry(std::string key, std::string value, uint16_t mark):
    key(key), value(value), totalSize(key.size()+value.size()+entryHeaderSize), mark(mark) {}
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

  uint32_t getSize() {
    return  totalSize;
  }

  void print_entry() {
    std::cout <<"key: "+key << " value: " << value << " mark: " << mark << std::endl;
  }

public:
  std::string key;
  std::string value;
  uint32_t totalSize; // 我们期望key是远远小于value的
  uint16_t mark;
};


#endif // ENTRY_H_
