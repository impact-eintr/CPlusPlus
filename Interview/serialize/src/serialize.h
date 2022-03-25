#ifndef SERIALIZE_H_
#define SERIALIZE_H_
#include <cstring>
#include <cstdint>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

template <typename SerializableType>
std::string serialize(SerializableType &a) {
  return a.serialize();
}

template <typename SerializableType>
int deserialize(std::string &str, SerializableType &a) {
  return a.deserialize(str);
}

#define DEF_BASIC_TYPE_SERIALIZE(Type)                                         \
  template <> std::string serialize(Type &b) {                                 \
    std::string ret;                                                           \
    ret.append((const char *)&b, sizeof(Type));                                \
    return ret;                                                                \
  }

#define DEF_BASIC_TYPE_DESERIALIZE(Type)                                       \
  template <> int deserialize(std::string &str, Type &b) {                     \
    memcpy(&b, str.data(), sizeof(Type));                                      \
    return sizeof(Type);                                                       \
  }

#define DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(Type)                         \
  DEF_BASIC_TYPE_SERIALIZE(Type)                                               \
  DEF_BASIC_TYPE_DESERIALIZE(Type)

DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(char)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(uint8_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(uint16_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(uint32_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(uint64_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(int8_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(int16_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(int32_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(int64_t)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(long long int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(unsigned long long int)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(float)
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(double)

// for C++ type std::string
template <> std::string serialize(std::string &s) {
  int len = s.size();
  std::string ret;
  ret.append(::serialize(len));
  ret.append(s.data(), len);
  return ret;
}

template <> int deserialize(std::string &str, std::string &s) {
  int len;
  ::deserialize(str, len);
  s = str.substr(sizeof(len), len);
  return sizeof(int) + len;
}

class OutStream {
public:
  OutStream() : os(std::ios::binary) {} // 构造函数

  // 重载 a << S
  template <typename SerializableType>
  OutStream &operator<<(SerializableType &a) {
    std::string x = ::serialize(a); // 调用全局的serialize函数
    os.write(x.data(), x.size());
    return *this;
  }

  // 重载 a << [S1, S2, S3, S3 ...]
  template <typename BasicType>
  OutStream &operator<<(std::vector<BasicType> &a) {
    int len = a.size();
    std::string x = ::serialize(len); // 序列化长度
    os.write(x.data(), x.size());

    for (int i = 0; i < len; ++i) {
      std::string item = ::serialize(a[i]);
      os.write(item.data(), item.size());
    }
    return *this;
  }

  // 重载 a << S1->S2->S3->...
  template <typename BasicType> OutStream &operator<<(std::list<BasicType> &a) {
    std::vector<BasicType> temp;
    std::copy(a.begin(), a.end(), std::back_inserter(temp));
    return this->operator<<(temp);
  }

  // 重载 a << (S1, S2, S3, ...)
  template <typename BasicType> OutStream &operator<<(std::set<BasicType> &a) {
    std::vector<BasicType> temp;
    std::copy(a.begin(), a.end(), std::back_inserter(temp));
    return this->operator<<(temp);
  }

  // 重载 a << (k1, v1), (k2, v2), (k3, v3), ...>
  template <typename BasicTypeA, typename BasicTypeB>
  OutStream &operator<<(std::map<BasicTypeA, BasicTypeB> &a) {
    std::vector<BasicTypeA> tempKey;
    std::vector<BasicTypeB> tempVal;

    typename std::map<BasicTypeA, BasicTypeB>::const_iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
      tempKey.push_back(it->first);
      tempVal.push_back(it->second);
    }

    this->operator<<(tempKey);
    return this->operator<<(tempVal);
  }

  std::string str() { return os.str(); }

public:
  std::ostringstream os;
};

class InStream {
public:
  InStream(std::string &s) : str(s), total(s.size()) {}

  template <typename SerializableType>
  InStream &operator>>(SerializableType &a) {
    int ret = ::deserialize(str, a);
    str = str.substr(ret);
    return *this;
  }

  template <typename BasicType>
  InStream &operator>>(std::vector<BasicType> &a) {
    int len = 0;
    int ret = ::deserialize(str, len);
    str = str.substr(ret);

    for (int i = 0; i < len; ++i) {
      BasicType item;
      int size = ::deserialize(str, item);
      str = str.substr(size);
      a.push_back(item);
    }
    return *this;
  }

  template <typename BasicType> InStream &operator>>(std::list<BasicType> &a) {
    std::vector<BasicType> temp;
    InStream &ret = this->operator>>(temp);
    if (temp.size() > 0) {
      std::copy(temp.begin(), temp.end(), std::back_inserter(a));
    }
    return ret;
  }

  template <typename BasicType> InStream &operator>>(std::set<BasicType> &a) {
    std::vector<BasicType> temp;
    InStream &ret = this->operator>>(temp);
    if (temp.size() > 0) {
      for (size_t i = 0; i < temp.size(); ++i) {
        a.insert(temp[i]);
      }
    }
    return ret;
  }

  template <typename BasicTypeA, typename BasicTypeB>
  InStream &operator>>(std::map<BasicTypeA, BasicTypeB> &a) {
    std::vector<BasicTypeA> tempKey;
    std::vector<BasicTypeB> tempVal;

    this->operator>>(tempKey);
    InStream &ret = this->operator>>(tempVal);

    if (tempKey.size() > 0 && tempVal.size() == tempKey.size()) {
      for (size_t i = 0; i < tempKey.size(); ++i) {
        a.insert(std::make_pair<BasicTypeA, BasicTypeB>(tempKey[i], tempVal[i]));
      }
    }
    return ret;
  }

  int size() {
    return total - str.size();
  }

protected:
  std::string str;
  int total;
};

class Serializable {
public:
  virtual std::string serialize() = 0;        // 纯虚函数
  virtual int deserialize(std::string &) = 0; // 纯虚函数
};

#endif // SERIALIZE_H_
