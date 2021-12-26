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

## GCC

``` c++
#include <cassert>
namespace jj05 {

enum { __ALIGN = 8 };                          // 区块的上调边界
enum { __MAX_BYTES = 128 };                    // 小区块的上限
enum { __NFREELISTS = __MAX_BYTES / __ALIGN }; // freelist的个数

template <bool threads, int inst> class __default_alloc_template {
private:
  static uint64_t ROUND_UP(uint64_t x) {
    return ((x + __ALIGN - 1) & (~(__ALIGN - 1)));
  };

private:
  union obj {
    union obj *free_list_link;
  };

private:
  static obj *volatile free_list[__NFREELISTS];
  static size_t FREELIST_INDEX(size_t bytes) {
    return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
  }

  static void *refill(size_t n);

  static char *chunk_alloc(size_t size, int &nobjs);

  // 空闲队列
  static char *start_free;
  static char *end_free;
  static size_t heap_size;

public:
  static void *allocate(size_t n) {
    assert(n > 0);

    obj *volatile *my_free_list;
    obj *result;

    if (n > (size_t)__MAX_BYTES) {
      return malloc(n);
    }

    // 查找合适的链表
    my_free_list = free_list + FREELIST_INDEX(n);
    result = *my_free_list;

    // 当前节点下没有已经分配好的内存
    if (result == nullptr) {
      void *r = refill(ROUND_UP(n));
      return r;
    }

    *my_free_list = result->free_list_link;
    return result;
  }

  static void deallocate(void *p, size_t n) {
    obj *q = (obj *)p;
    obj *volatile *my_free_list;

    if (n > (size_t)__MAX_BYTES) {
      free(p);
      return;
    }
    my_free_list = free_list + FREELIST_INDEX(n);
    q->free_list_link = *my_free_list;
    *my_free_list = q;
  }
  static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::chunk_alloc(size_t size,
                                                           int &nobjs) {
  char *result;
  size_t total_bytes = size * nobjs;
  size_t bytes_left = end_free - start_free;

  if (bytes_left >= total_bytes) { // [1] 剩余空间足够扩展内存池
    result = start_free;
    start_free += total_bytes;
    return result;
  } else if (bytes_left >= size) { // [2] 不足以按规格扩展内存池
                                   // 但足够本次申请使用 能扩展多少就扩展多少
    nobjs = bytes_left / size;
    total_bytes = size * nobjs;
    result = start_free;
    start_free += total_bytes;
    return result;
  } else { // [3] 连这次内存申请都不够
    size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
    if (bytes_left > 0) { // 重新部署当前pool剩余的空间到某一个链表下
      obj *volatile *my_free_list = free_list + FREELIST_INDEX(bytes_left);

      ((obj *)start_free)->free_list_link = *my_free_list;
      *my_free_list = (obj *)start_free;
    }

    // 申请一大块内存
    start_free =
        bytes_to_get + heap_size > 10000 ? 0 : (char *)malloc(bytes_to_get);

    if (0 == start_free) { // 申请失败
      obj *volatile *my_free_list;
      obj *p;

      // 遵循就近原则
      for (int i = size; i <= __MAX_BYTES; i += __ALIGN) {
        my_free_list = free_list + FREELIST_INDEX(i);
        p = *my_free_list;
        if (0 != p) {
          *my_free_list = p->free_list_link;
          // 将free_list内的目前第一块当成pool
          start_free = (char *)p;
          end_free =
              start_free + i; // 内存碎片? start_free + ((i/__ALIGN)+1)*__ALIGN
          return chunk_alloc(size, nobjs); // 这次再看够不够
        }
      }
      // pool_allocater已经无能为力
      end_free = 0;
      start_free = (char *)malloc(bytes_to_get);
    }
    heap_size += bytes_to_get;
    end_free = start_free + bytes_to_get;
    return chunk_alloc(size, nobjs); // 这次再看够不够
  }
}

template <bool threads, int inst>
void *__default_alloc_template<threads, inst>::refill(size_t n) {
  int nobjs = 20;
  char *chunk = chunk_alloc(n, nobjs);
  obj *volatile *my_free_list;
  obj *result;
  obj *current_obj;
  obj *next_obj;

  if (1 == nobjs) { // [2]中将nobjs设为1  这次分配只获得了一块内存
                    // 就不要构建free_list了
    return chunk;
  }

  // 构建free_list
  my_free_list = free_list + FREELIST_INDEX(n);
  result = (obj *)chunk;
  *my_free_list = next_obj = (obj *)(chunk + n); // n 是每个obj的大小

  for (int i = 1;; ++i) {
    current_obj = next_obj;
    next_obj = (obj *)((char *)next_obj + n);
    if (nobjs - 1 == i) {
      current_obj->free_list_link = nullptr;
      break;
    } else {
      current_obj->free_list_link = next_obj;
    }
  }
  return result;
}

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj
    *volatile __default_alloc_template<threads, inst>::free_list[__NFREELISTS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

typedef __default_alloc_template<false, 0> alloc;

void test_G29_alloc() {
  cout << "test_global_allocator_with_16_freelist().......... \n";

  void *p1 = alloc::allocate(120);
  void *p2 = alloc::allocate(72);
  void *p3 = alloc::allocate(60); //不是 8 倍數

  cout << p1 << ' ' << p2 << ' ' << p3 << endl;

  alloc::deallocate(p1, 120);
  alloc::deallocate(p2, 72);
  alloc::deallocate(p3, 60);

  cout << endl;
}

} // namespace jj05


```

# malloc/free

``` c++
#include <iostream>
#include <cstdint>

using namespace std;

int main(int argc, char *argv[]) {
  int b1[4]{1, 2, 3, 4}; // 16 align
  int b2[5]{5, 4, 3, 2, 1};

  cout << b1 << " " << b2 << endl;

  //cout << sizeof(int) << endl; // 4
  cout << *(int *)((uint64_t)&b2 - sizeof(int)) << endl;

  return 0;
}

```


``` c++
#include <iostream>
#include <cstdint>

using namespace std;

int main(int argc, char *argv[]) {

  int* p = (int *)malloc(4 * sizeof(int)); // p 在 stack上

  // 但以下数据在heap上
  p[0] = 1;
  p[1] = 1;
  p[2] = 1;
  p[3] = 1;

  free(p);

  return 0;
}

```

> const char * 与 char *const

``` c++
#include <iostream>
#include <cstdlib>
#include <cstdint>

using namespace std;

int main(int argc, char *argv[]) {

  char * const p = "hello world\0";
  const char * str = (char *)malloc(6);

  //p = str;
  //*(str+1) = 'l';
  str = p;

  cout << p << endl;
  cout << str << endl;

  return 0;
}

```


``` c++
#include <iostream>
#include <cstdlib>
#include <cstdint>

using namespace std;

int main(int argc, char *argv[]) {

  char * str = (char *)malloc(6);
  str = "hello\0";


// size = 16 = 8 * 2 byte = 0x0000 0010
//  0000 0000
//  0000 0000
//  0000 0000
//  0000 0000
//  0000 0000
//  0000 0000
//  '\0'
//  'o'
//  ========= 8 byte对齐
//  'l'
//  'l'
//  'e'
//  'h'
//  0000 0000
//  0000 0000
//  0000 0000
//  0001 0001

//  6 / 16 = 0.375

  cout << str << endl;

  return 0;
}

```

## heap 就是一个大数组

``` c++
extern uint64_t heap_start_vaddr;
extern uint64_t heap_end_vaddr;

#define HEAP_MAX_SIZE (4096 * 1024) // 4MB
extern uint8_t heap[HEAP_MAX_SIZE];
```

``` c++
#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include <alloca.h>
#include <cstdint>
#include <stdint.h>

extern uint64_t heap_start_vaddr;
extern uint64_t heap_end_vaddr;

#define HEAP_MAX_SIZE (4096 * 1024) // 4MB
extern uint8_t heap[HEAP_MAX_SIZE];

#define FREE (0)
#define ALLOCATED (1)
#define NIL (0)

#define MIN_EXPLICIT_FREE_LIST_BLOCKSIZE (16)
#define MIN_READBLACK_TREE_BLOCKSIZE (24)

// to allocate on physical page for heap(申请一个页表)
uint32_t extend_heap(uint32_t size);
void os_syscall_brk();

// round up
uint64_t round_up(uint64_t x, uint64_t n);

/*
 * operations for all blocks
 * */
// 获取/设置 blocksize
uint32_t get_blocksize(uint64_t header_vaddr);
void set_blocksize(uint64_t header_vaddr, uint32_t blocksize);

// 获取/设置 分配状态
uint32_t get_allocated(uint64_t header_vaddr);
void set_allocated(uint64_t header_vaddr, uint32_t allocated);

// 获取cookies
uint64_t get_payload(uint64_t vaddr);
uint64_t get_header(uint64_t vaddr);
uint64_t get_footer(uint64_t vaddr);

/*
 * operations for heap linked list
 * */

uint64_t get_nextheader(uint64_t vaddr);
uint64_t get_prevheader(uint64_t vaddr);

uint64_t get_prologue(); // 前言
uint64_t get_epilogue(); // 后记

uint64_t get_firstblock();
uint64_t get_lastblock();

int is_lastblock(uint64_t vaddr);
int is_firstblock(uint64_t vaddr);

// for free block as data structure
uint64_t get_field32_block_ptr(uint64_t header_vaddr, uint32_t min_blocksize,
                               uint32_t offset);
void set_field32_block_ptr(uint64_t header_vaddr, uint64_t block_ptr,
                           uint32_t min_blocksize, uint32_t offset);

// interface
int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t vaddr);

#endif // ALLOCATOR_H_

```

## block 的结构

``` c++
高位
 ____________________
|___blocksize__|_A/F_| <- footer
|                    |
|                    |
|      payload       |
|                    |
|____________________|
|___blocksize__|_A/F_| <- header

低位

// start_of_heap
// (heap_start_vaddr)| [4(padding)] ->[8(header+footer)] [...]

// end_of_heap
// (heap_start_vaddr)| [4(padding)] [8(header+footer)] [...] ->[header] |(heap_end_vaddr)
 
```


``` c++
#include "allocator.h"
#include <cassert>
#include <cstdint>
#include <iostream>

#define AF_BIT (0) // ALLOCATED / FREE
#define P8_BIT (1) // PREV BLOCK is a BLOCK 8
#define B8_BIT (2) // THIS BLOCK is 8 bytes


uint64_t round_up(uint64_t x, uint64_t n) { return n * ((x + n - 1) / n); }

// 设置第n位为1 H >> L
static void set_bit(uint64_t vaddr, int bit_offset) {
  uint32_t vector = 1 << bit_offset; // 000...1...000
  assert((vaddr & 0x3) == 0);        // 4对齐
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  *(uint32_t *)&heap[vaddr] |= vector;
}

// 将第n位清零 H >> L
static void reset_bit(uint64_t vaddr, int bit_offset) {
  uint32_t vector = 1 << bit_offset; // 000...1...000
  assert((vaddr & 0x3) == 0);        // 4对齐
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  *(uint32_t *)&heap[vaddr] &= (~vector); // 111...0...111
}

// 检查第n位是否为1 H >> L
static int is_bit_set(uint64_t vaddr, int bit_offset) {
  assert((vaddr & 0x3) == 0); // 4对齐
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  return (*(uint32_t *)&heap[vaddr] >> bit_offset) & 1;
}

static int is_block8(uint64_t vaddr) {
  if (vaddr == NIL || vaddr == get_epilogue()) {
    return 0;
  }
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());

  if (vaddr % 8 == 4) {
    // header
    if (is_bit_set(vaddr, B8_BIT) == 1) {
      return 1;
    }
  } else if (vaddr % 8 == 0) {
    // footer
    uint64_t next_header = vaddr + 4; // 下一个block的header_vaddr
    if (is_bit_set(next_header, B8_BIT) == 1) {
      return 1;
    }
  }
  return 0;
}

// start_of_heap
// (heap_start_vaddr)| [4(padding)] ->[8(header+footer)] [...]
uint64_t get_prologue() {
  assert(heap_end_vaddr > heap_start_vaddr);
  assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0); // 检查是否4k对齐
  assert(heap_start_vaddr % 4096 == 0);

  return heap_start_vaddr + sizeof(uint32_t);
}

// end_of_heap
// (heap_start_vaddr)| [4(padding)] [8(header+footer)] [...] ->[header]
// |(heap_end_vaddr)
uint64_t get_epilogue() {
  assert(heap_end_vaddr > heap_start_vaddr);
  assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0); // 检查是否4k对齐
  assert(heap_start_vaddr % 4096 == 0);

  return heap_end_vaddr - sizeof(uint32_t);
}

uint32_t get_blocksize(uint64_t header_vaddr) {
  if (header_vaddr == NIL) {
    return 0;
  }

  assert(get_prologue() <= header_vaddr &&
         header_vaddr <= get_epilogue()); // 在可用范围内
  assert((header_vaddr & 0x3) == 0x0);

  if (is_block8(header_vaddr) == 1) {
    return 8;
  } else {
    return (*((uint32_t *)&heap[header_vaddr]) &
            0xFFFFFFF8); // 000 prev_block_is_block_8| block_8| a/f
  }
}

void set_blocksize(uint64_t header_vaddr, uint32_t blocksize) {
  if (header_vaddr == NIL) {
    return;
  }
  assert(get_prologue() <= header_vaddr &&
         header_vaddr <= get_epilogue()); // 在可用范围内
  assert((header_vaddr & 0x3) == 0x0);
  assert((blocksize & 0x7) == 0x0);

  uint64_t next_header_vaddr;
  if (blocksize == 8) {
    // small block is special
    if (header_vaddr % 8 == 0) {
      // do not set footer of small block
      //  '\0'
      //  'a'
      //  'a'
      //  'a'
      //  0000 0000
      //  0000 0000
      //  0000 0000
      //  0100 0001 size == 8 这种就不设置footer了
      // reset to header
      header_vaddr = header_vaddr - 4;
    }
    next_header_vaddr = header_vaddr + 8;

    set_bit(header_vaddr, B8_BIT); // 设置该block的 8byte属性
    if (next_header_vaddr <= get_epilogue()) {
      set_bit(next_header_vaddr,
              P8_BIT); // 下一个block的前一个block是一个 BLOCK_8
    }

    if (get_allocated(header_vaddr) == FREE) {
      // free 8-byte does not set block size
      return;
    }
    // else, set header blocksize 8
  } else {
    if (header_vaddr % 8 == 4) {
      // header
      next_header_vaddr = header_vaddr + blocksize;
    } else {
      // footer
      next_header_vaddr = header_vaddr + 4;
    }
    reset_bit(header_vaddr, B8_BIT); // 当前block不是 BLOCK_8
    if (next_header_vaddr <= get_epilogue()) {
      reset_bit(next_header_vaddr,
                P8_BIT); // 下一个block的前一个block不是 BLOCK_8
    }
  }
  *(uint32_t *)&heap[header_vaddr] &= 0x00000007; // reset size 保留低3位
  *(uint32_t *)&heap[header_vaddr] |=
      blocksize; // set size blocksize 是8 对齐的 低3位是0
}

uint32_t get_allocated(uint64_t header_vaddr) {
  if (header_vaddr == NIL) {
    return ALLOCATED;
  }

  assert(get_prologue() <= header_vaddr &&
         header_vaddr <= get_epilogue()); // 在可用范围内
  assert((header_vaddr & 0x3) == 0x0);

  // 处理是footer的特殊情况 -> 该block是个 BLOCK_8
  // 没有footer不能直接取值需要前移到 header
  if (header_vaddr % 8 == 0) {
    // footer
    // check if 8-byte small block
    uint64_t next_header_vaddr = header_vaddr + 4;
    if (next_header_vaddr <= get_epilogue()) {
      // check P8 bit of next
      if (is_bit_set(next_header_vaddr, P8_BIT) == 1) {
        // current block is 8-byte, no footer. check header instead
        header_vaddr -= 4;
      }
      // else, current block has footer
    } else {
      // this is block is epilogue but it's 8X
      assert(0);
    }
  }

  return (*((uint32_t *)&heap[header_vaddr]) & 0x1);
}

void set_allocated(uint64_t header_vaddr, uint32_t allocated) {
  if (header_vaddr == NIL) {
    return;
  }
  assert(get_prologue() <= header_vaddr &&
         header_vaddr <= get_epilogue()); // 在可用范围内
  assert((header_vaddr & 0x3) == 0x0);

  if (header_vaddr % 8 == 0) {
    // check if 8-bytes small block
    uint64_t next_header_vaddr = header_vaddr + 4;

    if (next_header_vaddr <= get_epilogue()) {
      // check P8 bit of next
      if (is_bit_set(next_header_vaddr, P8_BIT) == 1) {
        header_vaddr -= 4;
      }
      // else, current blocks has footer
    } else {
      assert(0);
    }
  }
  *(uint32_t *)&heap[header_vaddr] &=
      0xFFFFFFFE; // reset allocated 保留所有高位
  *(uint32_t *)&heap[header_vaddr] |= (allocated & 0x1);
}

// 传入 void * 或者 header都可以
uint64_t get_payload(uint64_t vaddr) {
  if (vaddr == NIL) {
    return NIL;
  }
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  assert((vaddr & 0x3) == 0); // 必须是4对齐
  return round_up(vaddr, 8);
}

uint64_t get_header(uint64_t vaddr) {
  if (vaddr == NIL) {
    return NIL;
  }
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  assert((vaddr & 0x3) == 0); // 必须是4对齐
  return round_up(vaddr, 8) - 4;
}

uint64_t get_footer(uint64_t vaddr) {
  if (vaddr == NIL) {
    return NIL;
  }
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  assert(vaddr % 8 == 4);
  // vaddr can be:
  // 1. starting address of the block (8 * n + 4)
  // 2. starting address of the payload (8 * m)
  assert(vaddr % 8 == 4);

  uint64_t header_vaddr = get_header(vaddr);
  uint64_t footer_vaddr = header_vaddr + get_blocksize(header_vaddr) - 4;

  assert(get_firstblock() < footer_vaddr && footer_vaddr < get_epilogue());
  assert(footer_vaddr % 8 == 0);
  return footer_vaddr;
}

uint64_t get_nextheader(uint64_t vaddr) {
  if (vaddr == NIL || vaddr == get_epilogue()) {
    return NIL;
  }
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());

  uint64_t header_vaddr = get_header(vaddr);
  uint32_t block_size = get_blocksize(header_vaddr);

  uint64_t next_header_vaddr = header_vaddr + block_size;
  assert(get_firstblock() < next_header_vaddr && next_header_vaddr <= get_epilogue());

  return next_header_vaddr;
}

uint64_t get_prevheader(uint64_t vaddr) {
  if (vaddr == NIL || vaddr == get_prologue()) {
    return NIL;
  }
  assert(get_prologue() <= vaddr && vaddr <= get_epilogue());
  uint64_t header_vaddr = get_header(vaddr);
  uint64_t prev_header_vaddr;

  // check P8 bit 0010
  if (is_bit_set(header_vaddr, P8_BIT) == 1) {
    // prev block is a BLOCK_8
    prev_header_vaddr = header_vaddr - 8;
    return prev_header_vaddr;
  } else {
    // > 8
    uint64_t prev_footer_vaddr = header_vaddr - 4;
    uint32_t prev_blocksize = get_blocksize(prev_footer_vaddr);

    prev_header_vaddr = header_vaddr - prev_blocksize;
    assert(get_prologue() <= prev_header_vaddr && prev_header_vaddr < get_epilogue());
    assert(get_blocksize(prev_header_vaddr) == get_blocksize(prev_footer_vaddr));
    assert(get_allocated(prev_header_vaddr) == get_allocated(prev_footer_vaddr));

    return prev_header_vaddr;
  }
}

// 返回第一个有效block的header
// (start)| [4(padding)] [8(the_prologue_block)] ->[header...footer] [...] [header] |(end)
uint64_t get_firstblock() {
  assert(heap_end_vaddr > heap_start_vaddr);
  assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0); // 检查是否4k对齐
  assert(heap_start_vaddr % 4096 == 0);

  // 4 for the not in use
  // 8 for the prologue block
  return get_prologue() + 8;
}

// 返回最后一个有效block的header
// (start)| [4(padding)] [8(the_prologue_block)] [...] ->[header...footer] [header] |(end)
uint64_t get_lastblock() {
  assert(heap_end_vaddr > heap_start_vaddr);
  assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0); // 检查是否4k对齐
  assert(heap_start_vaddr % 4096 == 0);

  uint64_t epilogue_header = get_epilogue();
  return get_prevheader(epilogue_header);
}

int is_firstblock(uint64_t vaddr) {
  if (vaddr == NIL) {
    return 0;
  }
  // vaddr can be:
  // 1. starting address of the block (8 * n + 4)
  // 2. starting address of the payload (8 * m)
  assert(get_firstblock() <= vaddr && vaddr < get_epilogue());
  assert((vaddr & 0x3) == 0x0);

  uint64_t header_vaddr = get_header(vaddr);

  if (header_vaddr == get_firstblock()) {
    return 1;
  }
  return 0;
}

int is_lastblock(uint64_t vaddr) {
  if (vaddr == NIL) {
    return 0;
  }
  assert(get_firstblock() <= vaddr && vaddr < get_epilogue());
  assert((vaddr & 0x3) == 0x0);

  uint64_t header_vaddr = get_header(vaddr);
  uint32_t blocksize = get_blocksize(header_vaddr);

  if (header_vaddr + blocksize == get_epilogue()) {
    return 1;
  }
  return  0;
}

uint64_t get_field32_block_ptr(uint64_t header_vaddr, uint32_t min_blocksize,
                               uint32_t offset) {
  if (header_vaddr == NIL) {
    return NIL;
  }

  assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
  assert(header_vaddr % 8 == 4);
  assert(get_blocksize(header_vaddr) >= min_blocksize);

  assert(offset % 4 == 0);

  uint32_t vaddr_32 = *(uint32_t *)&heap[header_vaddr + offset];
  return (uint64_t)vaddr_32;
}

void set_field32_block_ptr(uint64_t header_vaddr, uint64_t block_ptr,
                           uint32_t min_blocksize, uint32_t offset) {
  if (header_vaddr == NIL) {
    return;
  }

  assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
  assert(header_vaddr % 8 == 4);
  assert(get_blocksize(header_vaddr) >= min_blocksize);

  assert(block_ptr == NIL ||
         (get_firstblock() <= block_ptr && block_ptr <= get_lastblock()));
  assert(block_ptr == NIL || block_ptr % 8 == 4);
  assert(block_ptr == NIL || get_blocksize(block_ptr) >= min_blocksize);

  assert(offset % 4 == 0);

  // actually a 32-bit pointer
  assert((block_ptr >> 32) == 0);
  *(uint32_t *)&heap[header_vaddr + offset] = (uint32_t)(block_ptr & 0xFFFFFFFF);
}

```

## malloc / free

``` c++
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "allocator.h"

#define IMPLICIT_FREE_LIST

uint64_t heap_start_vaddr;
uint64_t heap_end_vaddr;
uint8_t heap[HEAP_MAX_SIZE];

/* ------------------------------------- */
/*  Debugging and Correctness Checking   */
/* ------------------------------------- */

// 检查heap
void check_heap_correctness() {
  int linear_free_counter = 0;
  uint64_t p = get_firstblock();
  while (p != NIL && p <= get_lastblock()) {
    assert(p % 8 == 4);
    assert(get_firstblock() <= p && p <= get_lastblock());

    uint64_t f = get_footer(p);
    uint32_t blocksize = get_blocksize(p);
    if (blocksize != 8) {
      assert(get_blocksize(p) == get_blocksize(f));
      assert(get_allocated(p) == get_allocated(f));
    }

    // rule 1: block[0] ==> A/F
    // rule 2: block[-1] ==> A/F
    // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
    // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
    // these 4 rules ensures that
    // adjacent free blocks are always merged together
    // henceforth external fragmentation are minimized
    if (get_allocated(p) == FREE) {
      linear_free_counter += 1;
    } else {
      linear_free_counter = 0;
    }
    assert(linear_free_counter <= 1);

    p = get_nextheader(p);
  }
}

static void block_info_print(uint64_t h) {
  uint32_t a = get_allocated(h);
  uint32_t s = get_blocksize(h);
  uint64_t f = get_footer(h);

  uint32_t hv = *(uint32_t *)&heap[h];
  uint32_t fv = *(uint32_t *)&heap[f];

  uint32_t p8 = (hv >> 1) & 0x1;
  uint32_t b8 = (hv >> 2) & 0x1;
  uint32_t rb = (fv >> 1) & 0x1;

  printf("H:%lu,\tF:%lu,\tS:%u,\t(A:%u,RB:%u,B8:%u,P8:%u)\n", h, f, s, a, rb,
         b8, p8);
}

static void heap_blocks_print() {
  printf("============\nheap blocks:\n");
  uint64_t h = get_firstblock();
  int i = 0;
  while (i < (HEAP_MAX_SIZE / 8) && h != NIL && h < get_epilogue()) {
    block_info_print(h);
    h = get_nextheader(h);
  }
  printf("============\n");
}

/*================================*/
/*          向OS申请内存          */
/*================================*/
void os_syscall_brk() {}

uint32_t extend_heap(uint32_t size) {
  size = (uint32_t)round_up((uint64_t)size, 4096);
  if (heap_start_vaddr - heap_start_vaddr+ size <= HEAP_MAX_SIZE) {
    os_syscall_brk();  // 缺页异常
    heap_end_vaddr += size;
  } else {
    return 0;
  }
  uint64_t epilogue = get_epilogue();
  set_allocated(epilogue, ALLOCATED);
  set_blocksize(epilogue, 0);

  return size;
}

/* ------------------------------------- */
/*            各种不同的实现             */
/* ------------------------------------- */
// ===============Implicit Free List=================
#ifdef IMPLICIT_FREE_LIST
int implicit_list_initialize_free_block();
uint64_t implicit_list_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int implicit_list_insert_free_block(uint64_t free_header);
int implicit_list_delete_free_block(uint64_t free_header);
void implicit_list_check_free_block();
#endif

// ===============Explicit Free List=================
#ifdef EXPLICIT_FREE_LIST
int explicit_list_initialize_free_block();
uint64_t explicit_list_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int explicit_list_insert_free_block(uint64_t free_header);
int explicit_list_delete_free_block(uint64_t free_header);
void explicit_list_check_free_block();
#endif

// ===============Redblack Tree Free=================
#ifdef REDBLACK_TREE
int redblack_tree_initialize_free_block();
uint64_t redblack_tree_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int redblack_tree_insert_free_block(uint64_t free_header);
int redblack_tree_delete_free_block(uint64_t free_header);
void redblack_tree_check_free_block();
#endif

static int initialize_free_block()
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_initialize_free_block();
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_initialize_free_block();
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_initialize_free_block();
#endif
}

static uint64_t search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_search_free_block(payload_size, alloc_blocksize);
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_search_free_block(payload_size, alloc_blocksize);
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_search_free_block(payload_size, alloc_blocksize);
#endif
}

static int insert_free_block(uint64_t free_header)
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_insert_free_block(free_header);
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_insert_free_block(free_header);
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_insert_free_block(free_header);
#endif
}

static int delete_free_block(uint64_t free_header)
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_delete_free_block(free_header);
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_delete_free_block(free_header);
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_delete_free_block(free_header);
#endif
}

static void check_free_block()
{
#ifdef IMPLICIT_FREE_LIST
    implicit_list_check_free_block();
#endif

#ifdef EXPLICIT_FREE_LIST
    explicit_list_check_free_block();
#endif

#ifdef REDBLACK_TREE
    redblack_tree_check_free_block();
#endif
}

/* ------------------------------------- */
/*       Malloc and Free 的具体实现      */
/* ------------------------------------- */
static uint64_t merge_blocks_as_free(uint64_t low, uint64_t high) {
  assert(low % 8 == 4);
  assert(high % 8 == 4);
  assert(get_firstblock() <= low && low < get_lastblock());
  assert(get_firstblock() < high && high <= get_lastblock());
  assert(get_nextheader(low) == high);
  assert(get_prevheader(high) == low);

  // must merge as free
  uint32_t blocksize = get_blocksize(low) + get_blocksize(high);

  set_blocksize(low, blocksize);
  set_allocated(low, FREE);

  uint64_t footer = get_footer(low);
  set_blocksize(footer, blocksize);
  set_allocated(footer, FREE);

  return low;
}

// 传入的是某个block的header 尝试 allocate 后进行 split
// 返回payload_vaddr 不是 header
// [(request_blocksize) (leftsize)] <- 这是一个free block 即将 被分配
static uint64_t try_alloc_with_splitting(uint64_t block_vaddr, uint32_t request_blocksize) {
  if (request_blocksize < 8) {
    return NIL; // 上一部内存对齐失败了
  }

  uint64_t b = block_vaddr;
  uint32_t b_blocksize = get_blocksize(b);
  uint32_t b_allocated = get_allocated(b);

  if (b_allocated == FREE && b_blocksize >= request_blocksize) {
    // allocate this block
    delete_free_block(b);
    uint64_t left_footer = get_footer(b); // b 原本的footer

    // set the header
    set_allocated(b, ALLOCATED);
    set_blocksize(b, request_blocksize);

    // set the footer
    uint64_t b_footer = b + request_blocksize - 4;
    set_allocated(b_footer, ALLOCATED);
    set_blocksize(b_footer, request_blocksize);

    uint32_t left_size = b_blocksize - request_blocksize;
    if (left_size >= 8) {
      // split the left space
      uint64_t left_header = get_nextheader(b);
      set_allocated(left_header, FREE);
      set_blocksize(left_header, left_size);

      set_allocated(left_footer, FREE);
      set_blocksize(left_footer, left_size);

      assert(get_footer(left_header) == left_footer);
      // TODO 分析下面函数
      insert_free_block(left_header);
    }
    return get_payload(b);
  }
  return NIL;
}

static uint64_t try_extend_heap_to_alloc(uint32_t size) {
  // get the size to be added
  uint64_t old_last = get_lastblock();
  uint32_t last_allocated = get_allocated(old_last);
  uint32_t last_blocksize = get_blocksize(old_last);

  uint32_t to_request_from_OS = size;
  if (last_allocated == FREE) {
    // last block can help the request
    to_request_from_OS -= last_blocksize;
    delete_free_block(old_last); // 从free_list上摘除
  }

  uint64_t old_epilogue = get_epilogue();

  uint32_t os_allocated_size = extend_heap(to_request_from_OS); // 一开始只有4096
  if (os_allocated_size != 0) {
    assert(os_allocated_size >= 4096); // TODO 这里限制了最大申请量 后续处理一下
    assert(os_allocated_size % 4096 == 0);

    uint64_t payload_header = NIL;

    if (last_allocated == ALLOCATED) {
      // no need to merge
      // take place the old epilogue as new lastblock
      uint64_t new_last = old_epilogue;
      set_allocated(new_last, FREE);
      set_blocksize(new_last, os_allocated_size);

      // set the new footer
      uint64_t new_last_footer = get_footer(new_last);
      set_allocated(new_last_footer, FREE);
      set_blocksize(new_last_footer, os_allocated_size);
      insert_free_block(new_last);

      payload_header = new_last;
    } else {
      // need to merge
      set_allocated(old_last, FREE); // make sure the last block is free
      set_blocksize(old_last, last_blocksize + os_allocated_size);

      uint64_t last_footer = get_footer(old_last);
      set_allocated(last_footer, FREE);
      set_blocksize(last_footer, last_blocksize + os_allocated_size);
      insert_free_block(old_last);

      payload_header = old_last;
    }

    // try to allocate
    uint64_t payload_vaddr = try_alloc_with_splitting(payload_header, size);
    if (payload_header != NIL) {
      return payload_vaddr;
    }
  }

  if (last_allocated == FREE) {
    insert_free_block(old_last); // 把刚才取出来的freeblock放回去
  }
  return NIL;
}

#define DEBUG
/*--------------------------------------*/
/*            对外暴露的函数            */
/*--------------------------------------*/
int heap_init() {
  // reset all to 0
  for (int i = 0; i < HEAP_MAX_SIZE; i += 8) {
    *(uint64_t *)&heap[i] = 0;
  }

  heap_start_vaddr = 0;
  heap_end_vaddr = 4096;

  // A epilogue
  // F free block
  // A prologue

  // set the prologue block
  uint64_t prologue_header = get_prologue();
  set_allocated(prologue_header, ALLOCATED);
  set_blocksize(prologue_header, 8);

  uint64_t prologue_footer = prologue_header + 4;
  set_allocated(prologue_footer, ALLOCATED);
  set_blocksize(prologue_footer, 8);

  // set the epilogue block
  // it is a footer only
  uint64_t epilogue = get_epilogue();
  set_allocated(epilogue, ALLOCATED);
  set_blocksize(epilogue, 0);

  // set the block size & allocated of the only regular block
  uint64_t first_header = get_firstblock();
  set_allocated(first_header, FREE);
  set_blocksize(first_header, 4096 - 4 - 8 - 4);

  uint64_t first_footer = get_footer(first_header);
  set_allocated(first_footer, FREE);
  set_blocksize(first_footer, 4096 - 4 - 8 - 4);

  initialize_free_block();

#ifdef DEBUG
  heap_blocks_print();
#endif
  return 1;
}

uint64_t mem_alloc(uint32_t size) {
  assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

  uint32_t alloc_blocksize = 0;
  uint64_t payload_header = search_free_block(size, &alloc_blocksize); // 要size给alloc_blocksize
  uint64_t payload_vaddr = NIL;

  if (payload_header != NIL) {
#ifdef DEBUG
    printf("找到了空闲\n");
#endif
    payload_vaddr = try_alloc_with_splitting(payload_header, alloc_blocksize);
    assert(payload_vaddr != NIL);
  } else {
#ifdef DEBUG
    printf("没找到空闲\n");
#endif
    payload_vaddr = try_extend_heap_to_alloc(alloc_blocksize);
  }
#ifdef DEBUG
    check_heap_correctness();
    check_free_block();
    heap_blocks_print();
#endif
  return payload_vaddr;
}

void mem_free(uint64_t payload_vaddr) {
  if (payload_vaddr == NIL) {
    return;
  }

  // get_firstblock 返回的是 第一个block的header 不是 payload
  assert(get_firstblock() < payload_vaddr && payload_vaddr < get_epilogue());
  assert((payload_vaddr & 0x7) == 0x0);

  // request can be first or last block
  uint64_t req = get_header(payload_vaddr);
  uint64_t req_footer = get_footer(req); // for last block, it is 0

  // get the block_header and allocated of next & prev blocks
  uint64_t next = get_nextheader(req);
  uint64_t prev = get_prevheader(req);
  uint32_t next_allocated = get_allocated(next);
  uint32_t prev_allocated = get_allocated(prev);

  if (next_allocated == ALLOCATED && prev_allocated == ALLOCATED) {
    // case 1 [A|(A->F)|A]
    set_allocated(req, FREE);
    set_allocated(req_footer, FREE);

    insert_free_block(req);
  } else if (next_allocated == FREE && prev_allocated == ALLOCATED) {
    // case 2 [A|(A->F)|F|A] => [A|F+F|A]
    delete_free_block(next);

    uint64_t one_free = merge_blocks_as_free(req, next);

    insert_free_block(one_free);
  } else if (next_allocated == ALLOCATED && prev_allocated == FREE) {
    // case 3 [A|F|(A->F)|A] => [A|F+F|A]
    delete_free_block(prev);

    uint64_t one_free = merge_blocks_as_free(prev, req);

    insert_free_block(one_free);
  } else if (next_allocated == FREE && prev_allocated == FREE) {
    // case 4 [A|F|(A->F)|F|A]
    delete_free_block(prev);
    delete_free_block(next);

    uint64_t one_free = merge_blocks_as_free(merge_blocks_as_free(prev, req), next);

    insert_free_block(one_free);
  } else {
    assert(0);
  }
#ifdef DEBUG
  printf("释放空间\n");
    check_heap_correctness();
    check_free_block();
    heap_blocks_print();
#endif
}

```

## 隐式空闲链表

> 数据结构

``` c++
// 所谓的 内存链表管理都是建立在 heap[MAX_HEAP_SIZE] 上的
// 对于 implicit_list 而言
// [header...footer | header...footer | header...footer]
// 其实就是直接操作这个数组 使用 header/footer +/- blocksize 实现一个抽象的单向链表


```

> 实现

``` c++
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "allocator.h"
#include "algorithm.h"
#include "block_list.h"

// 所谓的 内存链表管理都是建立在 heap[MAX_HEAP_SIZE] 上的
// 对于 implicit_list 而言
// [header...footer | header...footer | header...footer]
// 其实就是直接操作这个数组 使用 header/footer +/- blocksize 实现一个抽象的单向链表

// 外部引入的 block_list
extern linkedlist_internal_t block_list;

// Manage block blocks
#define MIN_IMPLICIT_FREE_LIST_BLOCKSIZE (8)

int implicit_list_initialize_free_block()
{
  // init block block list
  block_list_init();
  return 1;
}

uint64_t implicit_list_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
{
  // search 8-byte block list
  if (payload_size <= 4 && block_list.count != 0) {
    // a block block and 8-byte list is not empty
    *alloc_blocksize = 8;
    return block_list.head;
  }
    
  uint32_t free_blocksize = round_up(payload_size, 8) + 4 + 4; // 8对齐+header_footer
  *alloc_blocksize = free_blocksize;

  // search the whole heap
  uint64_t b = get_firstblock();
  while (b <= get_lastblock()) {
    uint32_t b_blocksize = get_blocksize(b);
    uint32_t b_allocated = get_allocated(b);

    if (b_allocated == FREE && free_blocksize <= b_blocksize) {
      return b;
    } else {
      b = get_nextheader(b);
    }
  }

  return NIL;
}

int implicit_list_insert_free_block(uint64_t free_header)
{
  assert(free_header % 8 == 4);
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(get_allocated(free_header) == FREE);

  uint32_t blocksize = get_blocksize(free_header);
  assert(blocksize % 8 == 0);
  assert(blocksize >= 8);

  switch (blocksize)
  {
  case 8:
    block_list_insert(free_header);
    break;
        
  default:
    break;
  }

  return 1;
}

int implicit_list_delete_free_block(uint64_t free_header)
{
  assert(free_header % 8 == 4);
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(get_allocated(free_header) == FREE);

  uint32_t blocksize = get_blocksize(free_header);
  assert(blocksize % 8 == 0);
  assert(blocksize >= 8);

  switch (blocksize)
  {
  case 8:
    block_list_delete(free_header);
    break;
        
  default:
    break;
  }

  return 1;
}

void implicit_list_check_free_block()
{
  block_list_check_free_blocks();
}

```

## 显式空闲链表


> 数据结构

``` c++
// 所谓的 内存链表管理都是建立在 heap[MAX_HEAP_SIZE] 上的
// 对于 explicit_list 而言
// 是维护了如下的一个free_list
//
//   ____   ________________prev___________   ____________prev_________________   ______prev____
//       | V                               | V                                 | V
// [...header.F.footer | header.A.footer | header.F.footer | header.A.footer | header.F.footer...]
//  ___^ |_____________next________________^  |________________next____________^ |______next____
//
// 遍历的时候只找这个 free_list 找到一个block_size >= request_size


```

> 实现

``` c++
#include "algorithm.h"
#include "allocator.h"
#include "block_list.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 所谓的 内存链表管理都是建立在 heap[MAX_HEAP_SIZE] 上的
// 对于 explicit_list 而言
// 是维护了如下的一个free_list
//
//   ____   ________________prev___________   ____________prev_________________   ______prev____
//       | V                               | V                                 | V
// [...header.F.footer | header.A.footer | header.F.footer | header.A.footer | header.F.footer...]
//  ___^ |_____________next________________^  |________________next____________^ |______next____
//
// 遍历的时候只找这个 free_list 找到一个block_size >= request_size

extern linkedlist_internal_t block_list;

/* ------------------------------------- */
/*  Operations for List Block Structure  */
/* ------------------------------------- */

static int compare_nodes(uint64_t first, uint64_t second) {
  return !(first == second);
}

static int is_null_node(uint64_t node_id) { return node_id == NULL_ID; }

static uint64_t get_prevfree(uint64_t header_vaddr) {
  return get_field32_block_ptr(header_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE,
                               4);
}

uint64_t get_nextfree(uint64_t header_vaddr) {
  return get_field32_block_ptr(header_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE,
                               8);
}

static int set_prevfree(uint64_t header_vaddr, uint64_t prev_vaddr) {
  set_field32_block_ptr(header_vaddr, prev_vaddr,
                        MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 4);
  return 1;
}

static int set_nextfree(uint64_t header_vaddr, uint64_t next_vaddr) {
  set_field32_block_ptr(header_vaddr, next_vaddr,
                        MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 8);
  return 1;
}

// register the 5 functions above to be called by the linked list framework
static linkedlist_node_interface i_free_block = {
    .is_null_node = &is_null_node,
    .compare_nodes = &compare_nodes,
    .get_node_prev = &get_prevfree,
    .set_node_prev = &set_prevfree,
    .get_node_next = &get_nextfree,
    .set_node_next = &set_nextfree,
};

/* ------------------------------------- */
/*  Operations for Linked List           */
/* ------------------------------------- */

static int update_head(linkedlist_internal_t *this_pointer,
                       uint64_t block_vaddr) {
  if (this_pointer == NULL) {
    return 0;
  }

  assert(block_vaddr == NULL_ID ||
         (get_firstblock() <= block_vaddr && block_vaddr <= get_lastblock()));
  assert(block_vaddr == NULL_ID || block_vaddr % 8 == 4);
  assert(block_vaddr == NULL_ID ||
         get_blocksize(block_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

  this_pointer->head = block_vaddr;
  return 1;
}

// The explicit free linked list
linkedlist_internal_t explicit_list;

void explist_list_init() {
  explicit_list.head = NULL_ID;
  explicit_list.count = 0;
  explicit_list.update_head = &update_head;
}

uint64_t explicit_list_search(uint32_t free_blocksize) {
  // search explicit free list
  uint64_t b = explicit_list.head;
  uint32_t counter_copy = explicit_list.count;
  for (int i = 0; i < counter_copy; ++i) {
    uint32_t b_blocksize = get_blocksize(b);
    uint32_t b_allocated = get_allocated(b);

    if (b_allocated == FREE && free_blocksize <= b_blocksize) {
      return b;
    } else {
      b = get_nextfree(b);
    }
  }

  return NIL;
}

void explicit_list_insert(uint64_t free_header) {
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(free_header % 8 == 4);
  assert(get_blocksize(free_header) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
  assert(get_allocated(free_header) == FREE);

  linkedlist_internal_insert(&explicit_list, &i_free_block, free_header);
}

void explicit_list_delete(uint64_t free_header) {
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(free_header % 8 == 4);
  assert(get_blocksize(free_header) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
  // assert(get_allocated(free_header) == FREE);

  linkedlist_internal_delete(&explicit_list, &i_free_block, free_header);
  set_prevfree(free_header, NIL);
  set_nextfree(free_header, NIL);
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */

// from segregated list
extern void check_size_list_correctness(linkedlist_internal_t *list,
                                        linkedlist_node_interface *i_node,
                                        uint32_t min_size, uint32_t max_size);

static void explicit_list_print() {
  uint64_t p = explicit_list.head;
  printf("explicit free list <{%lu},{%lu}>:\n", explicit_list.head,
         explicit_list.count);
  for (int i = 0; i < explicit_list.count; ++i) {
    printf("<%lu:%u/%u> ", p, get_blocksize(p), get_allocated(p));
    p = get_nextfree(p);
  }
  printf("\n");
}

void check_block16_correctness() {
  check_size_list_correctness(&explicit_list, &i_free_block, 16, 16);
}

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

int explicit_list_initialize_free_block() {
  uint64_t first_header = get_firstblock();

  set_prevfree(first_header, first_header);
  set_nextfree(first_header, first_header);

  explist_list_init();
  explicit_list_insert(first_header);

  // init block block list
  block_list_init();

  return 1;
}

uint64_t explicit_list_search_free_block(uint32_t payload_size,
                                         uint32_t *alloc_blocksize) {
  // search 8-byte block list
  if (payload_size <= 4) {
    // a block block
    *alloc_blocksize = 8;

    if (block_list.count != 0) {
      // 8-byte list is not empty
      return block_list.head;
    }
  } else {
    *alloc_blocksize = round_up(payload_size, 8) + 4 + 4;
    assert((*alloc_blocksize) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
  }

  // search explicit free list
  return explicit_list_search(*alloc_blocksize);
}

int explicit_list_insert_free_block(uint64_t free_header) {
  assert(free_header % 8 == 4);
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(get_allocated(free_header) == FREE);

  uint32_t blocksize = get_blocksize(free_header);
  assert(blocksize % 8 == 0);
  assert(blocksize >= 8);

  switch (blocksize) {
  case 8:
    block_list_insert(free_header);
    break;

  default:
    explicit_list_insert(free_header);
    break;
  }

  return 1;
}

int explicit_list_delete_free_block(uint64_t free_header) {
  assert(free_header % 8 == 4);
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(get_allocated(free_header) == FREE);

  uint32_t blocksize = get_blocksize(free_header);
  assert(blocksize % 8 == 0);
  assert(blocksize >= 8);

  switch (blocksize) {
  case 8:
    block_list_delete(free_header);
    break;

  default:
    explicit_list_delete(free_header);
    break;
  }

  return 1;
}

void explicit_list_check_free_block() {
  block_list_check_free_blocks();
  check_size_list_correctness(&explicit_list, &i_free_block, 16, 0xFFFFFFFF);
}

```

## 红黑树

不会红黑树 现在先学抽象的内存管理 之后补上

# other issues

## 


## 


## 


## 



