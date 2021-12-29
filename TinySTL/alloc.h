#ifndef ALLOC_H_
#define ALLOC_H_

#include <cstddef>
#include <cstdlib>
#include <new>

#ifndef THROW_BAD_ALLOC
#define THROW_BAD_ALLOC throw std::bad_alloc
#endif

namespace STL {
// 空间配置器 以字节为单位分配
class malloc_alloc {
private:
  static void *oom_malloc(size_t);
  static void (*malloc_alloc_oom_handler)();

public:
  // 直接使用malloc
  static void *allocate(size_t n) {
    void *result = malloc(n);
    if (nullptr == result)
      result = oom_malloc(n); // 无法使用malloc 改用oom_malloc
    return result;
  }
  // 直接使用free
  static void deallocate(void *p) { free(p); }
  // 模仿C++的set_new_heandler()
  typedef void (*F)();
  static F set_malloc_handler(F f) {
    void (*old)() = malloc_alloc_oom_handler;
    malloc_alloc_oom_handler = f;
    return old;
  }
};

void (*malloc_alloc::malloc_alloc_oom_handler)() = 0;

void *malloc_alloc::oom_malloc(size_t n) {
  void (*my_malloc_handler)();
  void *result;

  while (true) {
    my_malloc_handler = malloc_alloc_oom_handler;
    if (nullptr == my_malloc_handler)
      THROW_BAD_ALLOC();
    (*my_malloc_handler)(); // 调用处理函数
    result = malloc(n);     // 再次尝试分配内存
    if (result)
      return result;
  }
}

enum { ALIGN = 8 };                         // 小区块的上调边界
enum { MAX_BYTES = 128 };                   // 小型区块的上限
enum { LEN_FREE_LIST = MAX_BYTES / ALIGN }; // free_list节点个数

// free_lists 节点
union FreeNode {
  union FreeNode *next; // 指向下一块区块
  char data[1];         // 本块内存首地址
};

// 基于内存池的allocator
class pool_alloc {
private:
  // 将bytes上调至ALIGN的倍数
  static size_t ROUND_UP(size_t bytes) {
    return (bytes + ALIGN - 1) & ~(ALIGN - 1);
  }

  // 16个节点的free_list
  static FreeNode *free_list[LEN_FREE_LIST];

  // 根据bytes大小，决定使用free_list的第n号区块
  static size_t FREE_LIST_INDEX(size_t bytes) {
    return (bytes + ALIGN - 1) / ALIGN - 1;
  }

  // allocate()中调用，返回大小为size的空间地址
  // 并可能将多个大小为size的其它区块填充到free_list中
  static void *refill(size_t size);

  // refill()中调用，配置一大块空间，可容纳n_nodes个大小为size的区块
  // 若空间不足，n_nodes可能会降低（传引用）
  static char *chunk_alloc(size_t size, int &n_nodes);

  // 内存池状态
  static char *start; // 内存池起始位置
  static char *end;   // 内存池结束位置
  static size_t
      heap_size; // chunk_alloc时如果从heap空间中获取内存给内存池，则记录这个heap空间大小

public:
  static void *allocate(size_t n);
  static void deallocate(void *p, size_t n);
};

// 初值
char *pool_alloc::start = nullptr;
char *pool_alloc::end = nullptr;
size_t pool_alloc::heap_size = 0;
FreeNode *pool_alloc::free_list[LEN_FREE_LIST] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0};

void *pool_alloc::allocate(size_t n) {
  // 大于MAX_BYTES就调用一级配置器
  if (n > static_cast<size_t>(MAX_BYTES)) {
    return malloc_alloc::allocate(n);
  }
  // 寻找free_list中适当的区块
  FreeNode **my_free_node = free_list + FREE_LIST_INDEX(n);
  FreeNode *result = *my_free_node;
  if (nullptr == result) {
    // 没找到可用的free_list，将n上调，重新填充并返回
    return refill(ROUND_UP(n));
  }
  *my_free_node = result->next;
  return result;
};

void pool_alloc::deallocate(void *p, size_t n) {
  // 大于MAX_BYTES就调用一级配置器
  if (n > static_cast<size_t>(MAX_BYTES)) {
    malloc_alloc::deallocate(p);
    return;
  }
  FreeNode *q = reinterpret_cast<FreeNode *>(p);
  FreeNode **my_free_node = free_list + FREE_LIST_INDEX(n);
  q->next = *my_free_node;
  *my_free_node = q;
}

// free_list无可用时调用，为free_list填充空间
// 新的空间取自内存池，取得20个新节点
// 若内存池空间不足，则获得节点数会＜20
void *pool_alloc::refill(size_t size) {
  // size已经上调至ALIGN的倍数
  int n_nodes = 20;

  // 取得n_nodes个区块作为free_list的新节点
  char *chunk = chunk_alloc(size, n_nodes);

  if (1 == n_nodes)
    return chunk; // 只获得一个区块，直接返回，没有为free_list填充新空间

  // 否则为free_list纳入新节点
  FreeNode **my_free_node = free_list + FREE_LIST_INDEX(size);
  // chunk空间中第0个返回，第1 ~ n_nodes - 1个填充进free_list并串接起来
  FreeNode *result =
      reinterpret_cast<FreeNode *>(chunk); // 选择一块size大小的空间返回
  // chunk中剩余空间填充进free_list
  FreeNode *current_node, *next_node;
  *my_free_node = next_node = reinterpret_cast<FreeNode *>(chunk + size);
  for (int i = 1;; ++i) {
    current_node = next_node;
    next_node = reinterpret_cast<FreeNode *>(
        reinterpret_cast<char *>(next_node) + size);
    if (n_nodes - 1 == i) {
      current_node->next = nullptr;
      break;
    } else {
      current_node->next = next_node;
    }
  }
  return result;
}

// 从内存池取空间给free_list使用，size已上调至ALIGN的倍数
char *pool_alloc::chunk_alloc(size_t size, int &n_nodes) {
  char *result;
  size_t total_bytes = size * n_nodes;
  size_t left_bytes = end - start; // 内存池剩余空间

  if (left_bytes >= total_bytes) {
    // 剩余空间满足需求量
    result = start;
    start += total_bytes;
    return result;
  } else if (left_bytes >= size) {
    // 剩余空间不满足需求量，但能提供大于等于1个区块
    n_nodes = left_bytes / size; // 修改n_nodes
    total_bytes = size * n_nodes;
    result = start;
    start += total_bytes;
    return result;
  } else {
    // 剩余空间不够提供1个区块
    // 准备从堆中获取2倍需求量+附加量的空间
    size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
    // 将内存池的残余零头配给free_list中适当的节点
    if (left_bytes > 0) {
      FreeNode **my_free_node = free_list + FREE_LIST_INDEX(left_bytes);
      reinterpret_cast<FreeNode *>(start)->next = *my_free_node;
      *my_free_node = reinterpret_cast<FreeNode *>(start);
    }
    // 配置heap空间，补充内存池
    start = (char *)malloc(bytes_to_get);

    // heap空间不足，malloc失败
    if (nullptr == start) {
      // 从free_list中获取"尚未使用且足够大(>= size)"的区块，分给内存池
      for (int i = size; i <= MAX_BYTES; i += ALIGN) {
        FreeNode **my_free_node = free_list + FREE_LIST_INDEX(i);
        FreeNode *p = *my_free_node;
        if (nullptr != p) { // free_list中有合适区块
          *my_free_node = p->next;
          start = reinterpret_cast<char *>(p);
          end = start + i;
          // 内存池中又获得一些空间，递归调用自己，重新从内存池中取空间
          // 同时修正n_nodes
          return chunk_alloc(size, n_nodes);
        }
      }
      // free_list中也无可用区块，调用一级配置器，求助out_of_memory机制
      end = nullptr;
      start = (char *)malloc_alloc::allocate(bytes_to_get);
    }

    // heap空间配置成功 or 使用一级配置器配置heap空间成功 时
    heap_size += bytes_to_get;
    end = start + bytes_to_get;
    // 内存池中又获得一些空间，递归调用自己，重新从内存池中取空间
    // 同时修正n_nodes
    return chunk_alloc(size, n_nodes);
  }
}

} // namespace STL

#endif // ALLOC_H_
