#ifndef ARENA_H_
#define ARENA_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {

// Arena在leveldb中它是一个内存池，它所作的工作十分简单，申请内存时
// 将申请到的内存块放入std::vector
// blocks_中，在Arena的生命周期结束后，统一释放掉所有申请到的内存
class Arena {
public:
  // BigThree
  Arena();
  Arena(const Arena&)=delete;
  Arena& operator=(const Arena&)=delete;
  ~Arena();

  char* Allocate(size_t bytes);

  char* AllocateAligned(size_t bytes);

  size_t MemoryUsage() const {
    return _memory_usage.load(std::memory_order_relaxed);
  }

private:
  char *AllocateFallback(size_t bytes); // 申请后后备空间
  char *AllocateNewBlock(size_t block_bytes);

  // Allocation state
  char *_alloc_ptr; // 每分配一个Block，记录当前可用的offset指针
  size_t _alloc_bytes_remaining; // 每分配一个Block，记录当前可用的bytes大小

  // Array of new[] allocated memory blocks
  std::vector<char*> _blocks; // 每次分配的内存都放入vector中

  std::atomic<size_t> _memory_usage; // 当前已经分配的内存容量
};

inline char* Arena::Allocate(size_t bytes) {
  assert(bytes > 0);
  if (bytes <= _alloc_bytes_remaining) {
    char* result = _alloc_ptr;
    _alloc_bytes_remaining -= bytes;
    return result;
  }
  return AllocateFallback(bytes);
}
}; // namespace leveldb


#endif // ARENA_H_
