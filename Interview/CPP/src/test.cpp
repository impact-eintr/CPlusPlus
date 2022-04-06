#include <iostream>
#include <list>

class C {
public:
  C() noexcept { std::cout << "C()" << std::endl; }
  C(const C &) noexcept { std::cout << "C(const C&)" << std::endl; }
  C(C &&) noexcept { std::cout << "C(C&&)" << std::endl; }
};

C fun1() {
  C c;
  // 简单返回对象一般有 NRVO
  return c;
}

C fun2() {
  C c;
  // std::move禁止 NRVO
  return std::move(c);
}

C fun3(bool f) {
  C c1, c2;
  //有分支一般无 NRVO
  return f ? c1 : c2;
}

class T {
public:
  T() noexcept{
    std::cout << "T()\n";
  }

  ~T() noexcept {
    std::cout << "~T()\n";
  }

  void add() {slot_.push_back(1);}
  void size() {std::cout << slot_.size() << std::endl;}
  std::list<int> clear();
private:
  std::list<int> slot_;
};

std::list<int> T::clear() {
  return std::move(slot_);
}

int main(int argc, char **argv) {
  T t;
  t.add();
  t.add();
  t.add();
  t.size();

  //std::list<int>&& rt = t.clear();
  std::list<int> rt = t.clear();

  for (auto it = rt.begin();it != rt.end();it++) {
    std::cout << *it << " ";
  }
  t.add();

  t.size();

  std::cout << std::is_rvalue_reference<decltype(rt)>::value << std::endl;

}
