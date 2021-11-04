#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

u_int64_t heap_start_address = (u_int64_t)0x7ffffff02;

void *mymalloc(u_int32_t size) {
  u_int64_t p = heap_start_address;

  while (1) {
    u_int32_t header_value = *(u_int32_t *)p;
    u_int32_t p_allocated = header_value & 0x1;
    u_int32_t p_block_szie = header_value & 0xFFFFFFFE;

    if (p_allocated == 0 && p_block_szie - sizeof(u_int32_t) >= size) {
      // split 将大的内存块分为 已分配和未分配
      // 先对齐
      u_int32_t request_block_size = 4 + size;
      if ((request_block_size & 0x7)!= 0) {
        request_block_size = (request_block_size+0x7) & (~0x7);
      }
      u_int64_t q = p + request_block_size;

      *(u_int32_t *)q = p_block_szie - request_block_size;

      // 更正p的值
      *(u_int32_t *)p = request_block_size;
      *(u_int32_t *)p = *(u_int32_t *)p | 0x1;

      return (void *)(p + sizeof(u_int32_t));
    } else {
      p = p + p_block_szie;
    }
  }
}

void myfree(void *p) {}

int main() {
  void *p = malloc(sizeof(int));
  u_int64_t header_addr = (u_int64_t)p - 4;
  u_int32_t header_value = *(u_int32_t *)header_addr;
  u_int32_t block_szie = header_value & 0xFFFFFFFE;
  int allocated = header_value & 0x1;

}
