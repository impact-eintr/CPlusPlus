#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <vector>

using namespace std;

namespace VariadicTemplates {
// 递归打印
void printX() {} // 参数为0的时候

template <typename T, typename... Types>
void printX(const T &firstArg, const Types &...args) {
  cout << firstArg << endl;
  printX(args...);
}

// Max/Min
int maximum(int n) { return n; }

template <typename... Args> int maximum(int n, Args... args) {
  return max(n, maximum(args...));
}

// 模拟tuple内部实现
template <typename... Values> class _tuple; // 这里表示_tuple是一个模板类

template <> class _tuple<> {
public:
  _tuple() {}
};

template <typename Head, typename... Tail> //不希望产生实参个数为0的变长参数模板
class _tuple<Head, Tail...> : private _tuple<Tail...> //继承自参数少一个的tuple
{
  typedef _tuple<Tail...> inherited;

protected:
  Head m_head;

public:
  _tuple() = default;
  _tuple(Head v, Tail... vtail) : m_head(v), inherited(vtail...) {}

  auto head() -> decltype(m_head) { return m_head; } //使用decltype进行类型推导
  //	Head head(){ return m_head; }
  inherited &tail() { return *this; } // return后转换为inherited
};
} // namespace VariadicTemplates

namespace Initializer_list {
void print(initializer_list<int> vals) {
  for (auto p = vals.begin(); p != vals.end(); ++p) {
    cout << *p << "\n";
  }
}

class P {
public:
  P(int a, int b) { cout << "P(int, int), a=" << a << ", b=" << b << endl; }

  P(initializer_list<int> initlist) {
    cout << "P(initializer_list<int>), value=";
    for (auto i : initlist) {
      cout << i << " ";
    }
    cout << "\n";
  }
};

}; // namespace Initializer_list

namespace Explicit {
class P {
public:
  P(int a, int b) { cout << "P(int a, int b)" << endl; }
  P() { cout << "P()" << endl; }

  P(initializer_list<int>) { cout << "P(initializer_list<int>)" << endl; }

  explicit P(int a, int b, int c) {
    cout << "explicit P(int a, int b, int c)" << endl;
  }
};

void fp(const P &){};
}; // namespace Explicit

namespace Default {
class Zoo {
public:
  Zoo(int i1, int i2) : d1(i1), d2(i2) {}
  Zoo() : d1(0), d2(0) {}
  Zoo(const Zoo &) = delete;
  Zoo(Zoo &&) = default;
  Zoo &operator=(const Zoo &) = default;
  Zoo &operator=(const Zoo &&) = delete;
  virtual ~Zoo() {}

private:
  int d1, d2;
};
}; // namespace Default

namespace temptempparam {
#define SIZE 3000000
template <typename T, template <class> class Container> class XCls {
private:
  Container<T> c;

public:
  XCls() { // ctors
    for (long i = 0; i < SIZE; ++i) {
      c.insert(c.end(), T());
    }
    cout << "构造完毕" << endl;
  }
};
}; // namespace temptempparam

template <typename T> using Vec = vector<T, allocator<T>>;

int main(int argc, char *argv[]) {
  temptempparam::XCls<Explicit::P, Vec> c1;
  return 0;
}
