#ifndef ENTRY_H_
#define ENTRY_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>

#include "serialize.h"

using namespace std;

extern const uint64_t entryHeaderSize;

class entry : Serializable
{
public:
  entry() = default;
  entry(std::string key, std::string value, uint16_t mark):
    key(key), value(value), keySize(key.size()), valueSize(value.size()), mark(mark) {}
  ~entry() {
    key.clear();
  }

  std::string serialize() {
    OutStream os;
    os << keySize << valueSize << mark << key << value;
    return os.str();
  }

  int deserialize(std::string &str) {
    InStream is(str);
    is >> keySize >> valueSize >> mark;
    return is.size();
  }

  uint64_t getSize() {
    return entryHeaderSize + uint64_t(keySize) + uint64_t(valueSize);
  }

  void print_entry() {
    std::cout <<"key: "+key << " value: " << value << " mark: " << mark << std::endl;
  }

public:
  std::string key;
  std::string value;
  uint32_t keySize;
  uint32_t valueSize;
  uint16_t mark;
};


#endif // ENTRY_H_
