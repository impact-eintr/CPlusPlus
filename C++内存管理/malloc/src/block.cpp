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
