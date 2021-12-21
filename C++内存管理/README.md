# c++ 内存管理

## 总览

![img](img/内存分配结构.png )

|分配|释放|类别|可否重载|
|:-:|:-:|:-:|:-:|
|malloc()|free()|c函数|No|
|new|delete|c++表达式|No|
|::operator new()|::operator delete()|c++函数|Yes|
|allocator<T>::allocate()|allocator<T>::deallocate()|stl|不可以，但可以自由设计并以之搭配任何容器|

# primitives
## new 与 delete

> new

``` c++
Complex* pc = new Complex(1, 2);

// 编译器转换为类似下面的代码
Complex *pc;
try {
    void* mem = operator new( sizeof(Complex) ); // 分配内存
    pc = static_cast<Complex*>(mem); // 强制转型
    pc->Complex::Cpmplex(1, 2); // 调用构造函数
} cache(std::bad::alloc) {
    // 如果alloaction失败就不执行constructor
}
```

``` c++
void *operator(size_t size, const std::nothrow_t&) _THROE0() {
    void *p;
    while((p = malloc(size)) == 0) {
        _TRY_BEGIN
        _CACHE(std::bad_alloc)
        return(0);
        _CACHE_END;
    }
    return(p);
}
```


> delete

``` c++
Complex* pc = new Complex(1, 2);
delete pc;
```

``` c++
pc->~Complex(); // 调用析构函数
operator delete(pc); // 释放内存
```

``` c++
void __cdecl operator delete(void* p) _THROW0()
{
    free(p);
}
```

## array new 与 array delete

``` c++
Complex* pca = new Complex[3];

delete[] pca; // 唤起3次
```


## placement new

- placement new 允许我们建构于 allocated memory中
- 没有所谓 placement delete,因为placement new 根本没分配 memory. 亦或称呼 operator delete(size, void*) 为placement delete

``` c++
#include <new>

char* buf = new char[sizeof(Complex) * 3];
Complex* pc = new(buf)Complex(1, 2);
...
delete [] buf;
```

`Complex* pc = new(buf)Complex(1, 2);` 被编译器转化为

``` c++
Complex *pc;
try {
    void* mem = operator new( sizeof(Complex), buf); // 不分配内存
    pc = static_cast<Complex*>(mem); // 强制转型
    pc->Complex::Cpmplex(1, 2); // 调用构造函数
} cache(std::bad::alloc) {
    // 如果alloaction失败就不执行constructor
}
```

``` c++
void *operator(size_t, void* loc) {
    return loc;
}
```


## 重载

![img](img/分配内存的途径.png )

### 重载 ::operator new/delete/new []/delete[]

``` c++
namespace jj01 {
class Foo {
public:
  Foo() : _id(0) {
    cout << "default ctor.this=" << this << " id=" << _id << endl;
  }

  Foo(int i) : _id(i) { cout << "ctor.this=" << this << " id=" << _id << endl; }

  ~Foo() { cout << "dctor.this=" << this << " id=" << _id << endl; }

  static void *operator new(size_t);
  static void operator delete(void *, size_t);
  static void *operator new[](size_t);
  static void operator delete[](void *, size_t);

private:
  int _id;
  long _data;
  string _str;
};

inline void *Foo::operator new(size_t size) {
  Foo *p = (Foo *)malloc(size);
  cout << "operator new" << endl;
  return p;
}

inline void *Foo::operator new[](size_t size) {
  Foo *p = (Foo *)malloc(size);
  cout << "operator new[]" << endl;
  return p;
}

inline void Foo::operator delete(void *pdead, size_t size) {
  cout << "operator delete" << endl;
  free(pdead);
}

inline void Foo::operator delete[](void *pdead, size_t size) {
  Foo *p = (Foo *)malloc(size);
  cout << "operator delete[]" << endl;
  free(pdead);
}

} // namespace jj01


```

### 重载 new()/delete()

我们可以重载clas member operator(),写出多个版本，前提是每一个版本声明都必须有独特的参数列, **第一个参数必须是 size_t**

`Foo* pf = new(300, 'c')Foo;`

``` c++
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


```

``` c++

int main(int argc, char *argv[]) {
  jj01::Foo start;
  jj01::Foo* p1 = new(100)jj01::Foo(1);
  return 0;
}
```

## 设计分配器

### per-class allocator 1

``` c++
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

namespace jj03 {}

int main(int argc, char *argv[]) {
  for (int i = 0; i < 100; i++) {
    jj02::Screen *p = new jj02::Screen(12);
    delete p;
  }
  return 0;
}

```

### per-class allocator 2

``` c++
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


```

### per-class allocator 3
> macro for static allocator(MFC)

``` c++
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


```

## new handler
当operator没有能力为我们分配出申请的memory 会抛出一个 std::bad_alloc_exception 

`new(nothrow)Foo;`

``` c++
typedef void(*new_handler)();
new_handler set_new_handler(new_handler p) throw();
```

设计良好的new handler只有两个选择
- 让更多memory可用
- 调用 abort() 或者 exit()

# std::allocator

## VC6



## GCC


## 



## 


# malloc/free

## 



## 



## 

# loki::allocator

## 

## 

# other allocators

## 

## 
