/*
** 作者 EINTR
** Github: https://github.com/impact-eintr
*/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t vaddr);

uint64_t heap_start_vaddr = 0;
uint64_t heap_end_vaddr = 0;

#define HEAP_MAX_SIZE (4096 * 8) // 8个页表的大小
uint8_t heap[HEAP_MAX_SIZE];

typedef struct {
} block_t;

// 向上对齐
static uint64_t round_up(uint64_t x, uint64_t align) {
  return ((x + align - 1) & (~(align - 1)));
}

static uint32_t get_blocksize(uint64_t header_vaddr) {
  assert(heap_start_vaddr <= header_vaddr &&
         header_vaddr <= heap_end_vaddr - 4);
  assert((header_vaddr & 0x3) == 0x0); // 4byte align
  uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
  return header_value & 0xFFFFFFF8;
}

static void set_blocksize(uint64_t header_vaddr, uint32_t blocksize) {
  assert(heap_start_vaddr <= header_vaddr &&
         header_vaddr <= heap_end_vaddr - 4);
  assert((header_vaddr & 0x3) == 0x0); // 4byte align
  assert((blocksize & 0x7) == 0x0);

  *(uint32_t *)&heap[header_vaddr] &= 0x00000007;
  *(uint32_t *)&heap[header_vaddr] |= blocksize;
}

static uint32_t get_allocated(uint64_t header_vaddr) {
  assert(heap_start_vaddr <= header_vaddr &&
         header_vaddr <= heap_end_vaddr - 4);
  assert((header_vaddr & 0x3) == 0x0); // 4byte align
  uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
  return header_value & 0x1;
}

static void set_allocated(uint64_t header_vaddr, uint32_t allocated) {
  assert(heap_start_vaddr <= header_vaddr && header_vaddr <= heap_end_vaddr);
  assert((header_vaddr & 0x3) == 0x0);

  *(uint32_t*)&heap[header_vaddr] &= 0xFFFFFFF8;
  *(uint32_t*)&heap[header_vaddr] |= (allocated & 0x1);

}

static uint64_t get_nextheader(uint64_t vaddr) {
  uint64_t header_vaddr = round_up(vaddr, 8) - 4;
  uint32_t block_size = get_blocksize(header_vaddr);
  uint64_t next_header_vaddr = header_vaddr - block_size;
  assert(heap_start_vaddr <= next_header_vaddr &&
         next_header_vaddr <= heap_end_vaddr - 12); // 4 + 8
  return next_header_vaddr;
}

static uint64_t get_prevheader(uint64_t vaddr) {
  uint64_t header_vaddr = round_up(vaddr, 8) - 4;
  assert(header_vaddr >= 16); // 4 + 8 + 4

  uint64_t prev_footer_vaddr = header_vaddr - 4;
  uint32_t prev_blocksize = get_blocksize(prev_footer_vaddr);

  uint64_t prev_header_vaddr = header_vaddr - prev_blocksize;
  assert(heap_start_vaddr <= prev_header_vaddr &&
         prev_header_vaddr <= heap_end_vaddr - 12); // 4 + 8
  return prev_header_vaddr;
}

int heap_check() {
  // rule 1: block[0] ==> A/F
  // rule 2: block[-1] ==> A/F
  // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
  // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
  // these 4 rules ensure that
  // adjacent free blocks are always merged together
  // henceforth external fragmentation are minimized
  return 0;
}

int heap_init() {
  heap_start_vaddr = 4;
  set_blocksize(heap_start_vaddr, 4096 - 8);
  set_allocated(heap_start_vaddr, 0);
  heap_end_vaddr = 4094 - 1; // we don not set a footer for the block
  return 0;
}

// size - request payload size
// return - the virtual adress of payload
uint64_t mem_alloc(uint32_t size) {

  uint32_t request_blocksize = round_up(size, 8) + 4 + 4; // header + payload + footer

  uint64_t vaddr = heap_start_vaddr;
  while(vaddr <= heap_end_vaddr) {
    // 遍历链表
    if ()
  }

}
