#ifndef PREDEFINED_H_
#define PREDEFINED_H_

#include <cstring>
#include <string>

namespace bpt {
#define BP_ORDER 50

struct key_t {
  char k[16];
  key_t(const std::string &str = "") {
    bzero(k, sizeof(k));
    strcpy(k, str.data());
  }
};

// TODO 这里之后设计一下
struct value_t {
  char name[256];
  int age;
  char email[256];
};

inline int keycmp(const key_t &a, const key_t &b) {
  int x = strlen(a.k) - strlen(b.k);
  return 0 == x ? strcmp(a.k, b.k) : x; // TODO 这里可能有bug?
}

#define OPERATOR_KEYCMP(type)                                                  \
  bool operator<(const key_t &l, const type &r) {                              \
    return keycmp(l, r.key) < 0;                                               \
  }                                                                            \
  bool operator<(const type &l, const key_t &r) {                              \
    return keycmp(l.key, r) < 0;                                               \
  }                                                                            \
  bool operator==(const key_t &l, const type &r) {                             \
    return keycmp(l, r.key) == 0;                                              \
  }                                                                            \
  bool operator==(const type &l, const key_t &r) {                             \
    return keycmp(l.key, r) == 0;                                              \
  }

} // namespace bpt
#endif // PREDEFINED_H_
