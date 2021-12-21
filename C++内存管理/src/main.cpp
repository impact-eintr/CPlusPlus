#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>

using namespace std;

namespace jj01 {
  class Bad{};
class Foo {
public:
  Foo() : _id(0) {
    cout << "default ctor.this=" << this << " id=" << _id << endl;
  }

  Foo(int i) : _id(i) {
    cout << "ctor.this=" << this << " id=" << _id << endl;
    throw Bad{};
  }

  ~Foo() { cout << "dctor.this=" << this << " id=" << _id << endl; }

  static void *operator new(size_t);
  static void *operator new(size_t, void *);
  static void *operator new(size_t, long);
  static void *operator new(size_t, long, char);

  static void operator delete(void *, size_t);
  static void operator delete(void *, void *);
  static void operator delete(void *, long);
  static void operator delete(void *, long, char);

  static void *operator new[](size_t);
  static void operator delete[](void *, size_t);

private:
  int _id;
  long _data;
  string _str;
};

inline void *Foo::operator new(size_t size) {
  Foo *p = (Foo *)malloc(size);
  cout << "operator new(size_t)" << endl;
  return p;
}

inline void *Foo::operator new(size_t size, void *start) {
  cout << "operator new(size_t, void*)" << endl;
  return start;
}

inline void *Foo::operator new(size_t size, long extra) {
  Foo *p = (Foo *)malloc(size + extra);
  cout << "operator new(size_t, long)" << endl;
  return p;
}

inline void *Foo::operator new(size_t size, long extra, char init) {
  Foo *p = (Foo *)malloc(size + extra);
  cout << "operator new(size_t, long, init)" << endl;
  return p;
}

inline void *Foo::operator new[](size_t size) {
  Foo *p = (Foo *)malloc(size);
  cout << "operator new[]" << endl;
  return p;
}

inline void Foo::operator delete(void *pdead, size_t size) {
  cout << "operator delete(void*, size_t)" << endl;
  free(pdead);
}

// 只有在构造函数中抛出异常的时候才会调用 placement delete
inline void Foo::operator delete(void *pdead, void *p) {
  cout << "operator delete(void*, void*)" << endl;
  free(pdead);
}

inline void Foo::operator delete(void *pdead, long extra) {
  cout << "operator delete(void*, long)" << endl;
  free(pdead);
}

inline void Foo::operator delete(void *pdead, long extra, char init) {
  cout << "operator delete(void*, long, char)" << endl;
  free(pdead);
}

inline void Foo::operator delete[](void *pdead, size_t size) {
  Foo *p = (Foo *)malloc(size);
  cout << "operator delete[]" << endl;
  free(pdead);
}

} // namespace jj01

namespace jj02 {
class Screen {
public:
  Screen(int x) : i(x){};
  int get() { return i; }

  void *operator new(size_t);
  void operator delete(void *, size_t);

private:
  int i;
  Screen *next;
  static Screen *freeStore;     // 第一块空闲空间的指针
  static const int screenChunk; // chunk : 一大块
};

Screen *Screen::freeStore = nullptr;
const int Screen::screenChunk = 24;

inline void *Screen::operator new(size_t size) {
  Screen *p;
  if (!freeStore) {
    // linked list 是空的 直接申请一大块
    size_t chunk = screenChunk * size;
    freeStore = p = reinterpret_cast<Screen *>(new char[chunk]);
    for (; p != &freeStore[screenChunk - 1]; ++p) {
      p->next = p + 1;
    }
    p->next = nullptr;
  }
  p = freeStore;
  freeStore = freeStore->next;
  cout << "申请后:" << freeStore << endl;
  return p;
}

inline void Screen::operator delete(void *p, size_t size) {
  // 将 deleted object指向list的前端
  (static_cast<Screen *>(p))->next = freeStore;
  freeStore = static_cast<Screen *>(p);
  cout << "归还后:" << freeStore << endl;
}
} // namespace jj02

namespace jj03 {
class Airplane {
private:
  struct AirplaneRep {
    unsigned long miles;
    char type;
  };

  // 匿名联合体
  union {
    AirplaneRep rep; // 这个针对使用中的object
    Airplane *next;  // 这个针对 free list 上的object
  };

  static const int BLOCK_SIZE;
  static Airplane *headOfFreeList;

public:
  unsigned long getMails() { return rep.miles; }

  char getType() { return rep.type; }

  void set(unsigned long m, char t) {
    rep.miles = m;
    rep.type = t;
  }

  static void *operator new(size_t);
  static void operator delete(void *, size_t);
};

const int Airplane::BLOCK_SIZE = 512;
Airplane *Airplane::headOfFreeList;

inline void *Airplane::operator new(size_t size) {
  if (size != sizeof(Airplane)) {
    return ::operator new(size);
  }

  Airplane *p = headOfFreeList;
  if (p)
    headOfFreeList = p->next;
  else {
    Airplane *newBlock =
        static_cast<Airplane *>(::operator new(BLOCK_SIZE * sizeof(Airplane)));

    for (int i = 1; i < BLOCK_SIZE - 1; ++i) {
      newBlock[i].next = &newBlock[i + 1];
    }
    newBlock[BLOCK_SIZE - 1].next = nullptr;
    p = newBlock;
    headOfFreeList = &newBlock[1];
  }
  return p;
}

inline void Airplane::operator delete(void *p, size_t size) {
  if (p == nullptr)
    return;
  if (size != sizeof(Airplane)) {
    ::operator delete(p);
    return;
  }
  Airplane *q = static_cast<Airplane *>(p);
  q->next = headOfFreeList;
  headOfFreeList = q;
}

} // namespace jj03

#include <complex>
namespace jj04 {
class allocator {
private:
  struct obj {
    struct obj *next; // embedded pointer
  };

  obj *freeStore = nullptr;
  const int CHUNK = 5;

public:
  void *allocate(size_t);
  void deallocate(void *, size_t);
};

inline void *allocator::allocate(size_t size) {
  obj *p;
  if (!freeStore) {
    size_t chunk = CHUNK * size;
    freeStore = p = (obj *)malloc(chunk);

    for (int i = 0; i < (CHUNK - 1); ++i) {
      p->next = (obj *)((char *)p + size);
      p = p->next;
    }
    p->next = nullptr;
  }
  p = freeStore;
  freeStore = freeStore->next;
  return p;
}

inline void allocator::deallocate(void *p, size_t size) {
  ((obj *)p)->next = freeStore;
  freeStore = (obj *)p; // 将准备归还的内存嵌入空闲链表的头部
}

#define DECLEAR_POOL_ALLOC()                                                   \
public:                                                                        \
  void *operator new(size_t size) { return myAllocator.allocate(size); }       \
  void operator delete(void *p) { myAllocator.deallocate(p, 0); }              \
                                                                               \
protected:                                                                     \
  static allocator myAllocator;

#define IMPLEMENT_POOL_ALLOC(class_name) allocator class_name::myAllocator;

class Foo {
  DECLEAR_POOL_ALLOC()
public:
  long L;
  string str;
  static allocator myAlloc;

public:
  Foo(long l) : L(l) {}
};
IMPLEMENT_POOL_ALLOC(Foo)

class Goo {
  DECLEAR_POOL_ALLOC()
public:
public:
  complex<double> c;
  string str;
  static allocator myAlloc;

public:
  Goo(const complex<double> &x) : c(x) {}
};
IMPLEMENT_POOL_ALLOC(Goo)

} // namespace jj04

namespace vc6 {

}

int main(int argc, char *argv[]) {
  jj01::Foo start;
  jj01::Foo* p1 = new(100)jj01::Foo(1);
  return 0;
}
