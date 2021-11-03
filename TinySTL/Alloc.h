#ifndef ALLOC_H_
#define ALLOC_H_

#include <cstdlib>

namespace TinySTL{
  // 空间配置器 以字节为单位分配
  class alloc{
  private:
    enum EAlign {ALIGN = 8}; // 小型区块的上调边界
    enum EMaxBytes {MAXBYTES = 128}; // 小型区块的上限 超过的区块由malloc分配


  };
}

#endif // ALLOC_H_
