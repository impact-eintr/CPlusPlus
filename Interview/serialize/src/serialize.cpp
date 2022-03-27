#include <iostream>

#include "serialize.h"

class entry : Serializable
{
public:
  entry() = default;
  entry(std::string key, std::string value, uint16_t mark):
    key(key), value(value), keySize(key.size()), valueSize(value.size()), mark(mark) {}

  virtual std::string serialize() {
    OutStream os;
    os << keySize << valueSize << mark << key << value;
    return os.str();
  }

  virtual int deserialize(std::string &str) {
    InStream is(str);
    //is >> keySize >> valueSize >> mark >> key >> value;
    is >> keySize >> valueSize >> mark;
    return is.size();
  }

  void print_entry() {
    std::cout << key << " " << value << " " << mark << " "<< std::endl;
  }

public:
  std::string key;
  std::string value;
  uint32_t keySize;
  uint32_t valueSize;
  uint16_t mark;
};

void basicTypeTest() {
  char x = 'a';
  short a = 1;
  int b = 2;
  long c = 3;
  float d = 4;
  long long e = 5;

  OutStream os;
  os << x << a << b << c << d << e;

  std::cout << os.str() <<std::endl;
  std::string serializestr = os.str();

  char x1;
  short a1;
  int b1;
  long c1;
  float d1;
  long long e1;

  InStream is(serializestr);
  is >> x1 >> a1 >> b1 >> c1 >> d1 >> e1;
  std::cout << x1 << a1 << b1 << c1 << d1 << e1 << std::endl;
}

int main() {
  entry e1("k1", "v1", 1);
  e1.print_entry();
  std::string ss = e1.serialize();
  for (int i = 0;i < ss.size();++i) {
    printf("%d", ss[i]);
  }
  std::cout << ss.size() << std::endl;

  entry e2;
  e2.deserialize(ss);
  e2.print_entry();

  e2.key = "k2";
  e2.value = "v2";
  e2.mark = 0;
  ss = e2.serialize();

  e1.deserialize(ss);

  e1.print_entry();
}
