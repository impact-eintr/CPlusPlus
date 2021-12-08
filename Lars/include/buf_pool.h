#ifndef BUF_POOL_H_
#define BUF_POOL_H_

#include "io_buf.h"
#include <ext/hash_map>

// std容器要求元素元素具有 object type，引用不是 object type。
// typedef __gnu_cxx::hash_map<int, io_buf&> pool_t; error
typedef __gnu_cxx::hash_map<int, io_buf *> pool_t;

enum MSM_CAP {
  m4K = 4096,
  m16K = 16384,
  m64K = 65536,
  m256K = 262144,
  m1M = 1048576,
  m4M = 4194304,
  m8M = 8388608
};

// 总内存池最大限制 单位是Kb 所以目前限制是 5GB
#define EXTRA_MEM_LIMIT (5U * 1024 * 1024)

// 定义buffer内存池
class buf_pool {
public:
  // Magic Static
  // 如果当变量在初始化的时候，并发同时进入声明语句，并发线程将会阻塞等待初始化结束。
  static buf_pool &instance() {
    static buf_pool _instance;
    return _instance;
  }

  // 开辟一个io_buf
  io_buf *alloc_buf(int N);
  io_buf *alloc_buf() { return alloc_buf(m4K); }
  // 重置一个io_buf
  void revert(io_buf *buffer);

private:
  buf_pool();
  buf_pool(const buf_pool &)=delete;
  const buf_pool &operator=(const buf_pool &)=delete;

  // 所有buffer的一个map集合句柄
  pool_t _pool;
  // 总buffer池的内存大小 单位为KB
  uint64_t _total_mem;
  // 用户保护内存池链表修改的互斥锁
  static pthread_mutex_t _mutex;
};

#endif // BUF_POOL_H_
