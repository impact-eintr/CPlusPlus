#ifndef CONSTRUCT_H_
#define CONSTRUCT_H_

#include <new>
#include <utility>

#include "type_traits.h"

namespace STL {
// construct
template <class T1, class... Args> void construct(T1 *p, Args &&...args) {
  new (static_cast<void *>(p)) T1(std::forward<Args>(args)...);
}

// destory  version 1
template <class T> void destroy(T *pointer) { pointer->~T(); }

// destory  version 2
template <class ForwardIterator>
void destroy(ForwardIterator first, ForwardIterator last) {
  typedef typename STL::has_trivial_destructor<ForwardIterator> trivial_dtor;
  _destroy(first, last, trivial_dtor());
}

template <class ForwardIterator>
inline void _destroy(ForwardIterator, ForwardIterator, STL::true_type) {}

template <class ForwardIterator>
inline void _destroy(ForwardIterator first, ForwardIterator last,
                     STL::false_type) {
  for (; first < last; ++first) {
    destroy(&*first);
  }
}

} // namespace STL

#endif // CONSTRUCT_H_
