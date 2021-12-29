#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include "alloc.h"
#include "construct.h"
#include <cstddef>

namespace STL {
// 空间分配器 allocator
template <class T, class Alloc> class allocator {
public:
  using value_type = T; // 的另一种typedef
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;

  // rebind allocator of type U
  template <class U> struct rebind { typedef allocator<U, Alloc> other; };

  static pointer allocate() { return (T *)Alloc::allocate(sizeof(T)); }
  static pointer allocate(size_t n) {
    if (n > max_size())
      THROW_BAD_ALLOC();
    return 0 == n ? 0 : (T *)Alloc::allocate(sizeof(T) * n);
  }

  static void deallocate(T *p) { Alloc::deallocate(p, sizeof(T)); }
  static void deallocate(T *p, size_t n) {
    if (0 != n)
      Alloc::deallocate(p, sizeof(T) * n);
  }

  static size_type max_size() { return size_t(-1) / sizeof(value_type); }
};
} // namespace STL

#endif // ALLOCATOR_H_
