#ifndef ALLOC_H_
#define ALLOC_H_

#include <cstdlib>

namespace TinySTL{
  // 空间配置器 以字节为单位分配
  class alloc{
  private:
    enum Ealign {ALIGN = 8}; // 小型去快的上调边界
  };
}

#endif // ALLOC_H_
