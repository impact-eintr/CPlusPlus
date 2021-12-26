#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "allocator.h"

#define IMPLICIT_FREE_LIST

uint64_t heap_start_vaddr;
uint64_t heap_end_vaddr;
uint8_t heap[HEAP_MAX_SIZE];

/* ------------------------------------- */
/*  Debugging and Correctness Checking   */
/* ------------------------------------- */

// 检查heap
void check_heap_correctness() {
  int linear_free_counter = 0;
  uint64_t p = get_firstblock();
  while (p != NIL && p <= get_lastblock()) {
    assert(p % 8 == 4);
    assert(get_firstblock() <= p && p <= get_lastblock());

    uint64_t f = get_footer(p);
    uint32_t blocksize = get_blocksize(p);
    if (blocksize != 8) {
      assert(get_blocksize(p) == get_blocksize(f));
      assert(get_allocated(p) == get_allocated(f));
    }

    // rule 1: block[0] ==> A/F
    // rule 2: block[-1] ==> A/F
    // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
    // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
    // these 4 rules ensures that
    // adjacent free blocks are always merged together
    // henceforth external fragmentation are minimized
    if (get_allocated(p) == FREE) {
      linear_free_counter += 1;
    } else {
      linear_free_counter = 0;
    }
    assert(linear_free_counter <= 1);

    p = get_nextheader(p);
  }
}

static void block_info_print(uint64_t h) {
  uint32_t a = get_allocated(h);
  uint32_t s = get_blocksize(h);
  uint64_t f = get_footer(h);

  uint32_t hv = *(uint32_t *)&heap[h];
  uint32_t fv = *(uint32_t *)&heap[f];

  uint32_t p8 = (hv >> 1) & 0x1;
  uint32_t b8 = (hv >> 2) & 0x1;
  uint32_t rb = (fv >> 1) & 0x1;

  printf("H:%lu,\tF:%lu,\tS:%u,\t(A:%u,RB:%u,B8:%u,P8:%u)\n", h, f, s, a, rb,
         b8, p8);
}

static void heap_blocks_print() {
  printf("============\nheap blocks:\n");
  uint64_t h = get_firstblock();
  int i = 0;
  while (i < (HEAP_MAX_SIZE / 8) && h != NIL && h < get_epilogue()) {
    block_info_print(h);
    h = get_nextheader(h);
  }
  printf("============\n");
}

/*================================*/
/*          向OS申请内存          */
/*================================*/
void os_syscall_brk() {}

uint32_t extend_heap(uint32_t size) {
  size = (uint32_t)round_up((uint64_t)size, 4096);
  if (heap_start_vaddr - heap_start_vaddr+ size <= HEAP_MAX_SIZE) {
    os_syscall_brk();  // 缺页异常
    heap_end_vaddr += size;
  } else {
    return 0;
  }
  uint64_t epilogue = get_epilogue();
  set_allocated(epilogue, ALLOCATED);
  set_blocksize(epilogue, 0);

  return size;
}

/* ------------------------------------- */
/*            各种不同的实现             */
/* ------------------------------------- */
// ===============Implicit Free List=================
#ifdef IMPLICIT_FREE_LIST
int implicit_list_initialize_free_block();
uint64_t implicit_list_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int implicit_list_insert_free_block(uint64_t free_header);
int implicit_list_delete_free_block(uint64_t free_header);
void implicit_list_check_free_block();
#endif

// ===============Explicit Free List=================
#ifdef EXPLICIT_FREE_LIST
int explicit_list_initialize_free_block();
uint64_t explicit_list_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int explicit_list_insert_free_block(uint64_t free_header);
int explicit_list_delete_free_block(uint64_t free_header);
void explicit_list_check_free_block();
#endif

// ===============Redblack Tree Free=================
#ifdef REDBLACK_TREE
int redblack_tree_initialize_free_block();
uint64_t redblack_tree_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize);
int redblack_tree_insert_free_block(uint64_t free_header);
int redblack_tree_delete_free_block(uint64_t free_header);
void redblack_tree_check_free_block();
#endif

static int initialize_free_block()
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_initialize_free_block();
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_initialize_free_block();
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_initialize_free_block();
#endif
}

static uint64_t search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_search_free_block(payload_size, alloc_blocksize);
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_search_free_block(payload_size, alloc_blocksize);
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_search_free_block(payload_size, alloc_blocksize);
#endif
}

static int insert_free_block(uint64_t free_header)
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_insert_free_block(free_header);
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_insert_free_block(free_header);
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_insert_free_block(free_header);
#endif
}

static int delete_free_block(uint64_t free_header)
{
#ifdef IMPLICIT_FREE_LIST
    return implicit_list_delete_free_block(free_header);
#endif

#ifdef EXPLICIT_FREE_LIST
    return explicit_list_delete_free_block(free_header);
#endif

#ifdef REDBLACK_TREE
    return redblack_tree_delete_free_block(free_header);
#endif
}

static void check_free_block()
{
#ifdef IMPLICIT_FREE_LIST
    implicit_list_check_free_block();
#endif

#ifdef EXPLICIT_FREE_LIST
    explicit_list_check_free_block();
#endif

#ifdef REDBLACK_TREE
    redblack_tree_check_free_block();
#endif
}

/* ------------------------------------- */
/*       Malloc and Free 的具体实现      */
/* ------------------------------------- */
static uint64_t merge_blocks_as_free(uint64_t low, uint64_t high) {
  assert(low % 8 == 4);
  assert(high % 8 == 4);
  assert(get_firstblock() <= low && low < get_lastblock());
  assert(get_firstblock() < high && high <= get_lastblock());
  assert(get_nextheader(low) == high);
  assert(get_prevheader(high) == low);

  // must merge as free
  uint32_t blocksize = get_blocksize(low) + get_blocksize(high);

  set_blocksize(low, blocksize);
  set_allocated(low, FREE);

  uint64_t footer = get_footer(low);
  set_blocksize(footer, blocksize);
  set_allocated(footer, FREE);

  return low;
}

// 传入的是某个block的header 尝试 allocate 后进行 split
// 返回payload_vaddr 不是 header
// [(request_blocksize) (leftsize)] <- 这是一个free block 即将 被分配
static uint64_t try_alloc_with_splitting(uint64_t block_vaddr, uint32_t request_blocksize) {
  if (request_blocksize < 8) {
    return NIL; // 上一部内存对齐失败了
  }

  uint64_t b = block_vaddr;
  uint32_t b_blocksize = get_blocksize(b);
  uint32_t b_allocated = get_allocated(b);

  if (b_allocated == FREE && b_blocksize >= request_blocksize) {
    // allocate this block
    delete_free_block(b);
    uint64_t left_footer = get_footer(b); // b 原本的footer

    // set the header
    set_allocated(b, ALLOCATED);
    set_blocksize(b, request_blocksize);

    // set the footer
    uint64_t b_footer = b + request_blocksize - 4;
    set_allocated(b_footer, ALLOCATED);
    set_blocksize(b_footer, request_blocksize);

    uint32_t left_size = b_blocksize - request_blocksize;
    if (left_size >= 8) {
      // split the left space
      uint64_t left_header = get_nextheader(b);
      set_allocated(left_header, FREE);
      set_blocksize(left_header, left_size);

      set_allocated(left_footer, FREE);
      set_blocksize(left_footer, left_size);

      assert(get_footer(left_header) == left_footer);
      // TODO 分析下面函数
      insert_free_block(left_header);
    }
    return get_payload(b);
  }
  return NIL;
}

static uint64_t try_extend_heap_to_alloc(uint32_t size) {
  // get the size to be added
  uint64_t old_last = get_lastblock();
  uint32_t last_allocated = get_allocated(old_last);
  uint32_t last_blocksize = get_blocksize(old_last);

  uint32_t to_request_from_OS = size;
  if (last_allocated == FREE) {
    // last block can help the request
    to_request_from_OS -= last_blocksize;
    delete_free_block(old_last); // 从free_list上摘除
  }

  uint64_t old_epilogue = get_epilogue();

  uint32_t os_allocated_size = extend_heap(to_request_from_OS); // 一开始只有4096
  if (os_allocated_size != 0) {
    assert(os_allocated_size >= 4096); // TODO 这里限制了最大申请量 后续处理一下
    assert(os_allocated_size % 4096 == 0);

    uint64_t payload_header = NIL;

    if (last_allocated == ALLOCATED) {
      // no need to merge
      // take place the old epilogue as new lastblock
      uint64_t new_last = old_epilogue;
      set_allocated(new_last, FREE);
      set_blocksize(new_last, os_allocated_size);

      // set the new footer
      uint64_t new_last_footer = get_footer(new_last);
      set_allocated(new_last_footer, FREE);
      set_blocksize(new_last_footer, os_allocated_size);
      insert_free_block(new_last);

      payload_header = new_last;
    } else {
      // need to merge
      set_allocated(old_last, FREE); // make sure the last block is free
      set_blocksize(old_last, last_blocksize + os_allocated_size);

      uint64_t last_footer = get_footer(old_last);
      set_allocated(last_footer, FREE);
      set_blocksize(last_footer, last_blocksize + os_allocated_size);
      insert_free_block(old_last);

      payload_header = old_last;
    }

    // try to allocate
    uint64_t payload_vaddr = try_alloc_with_splitting(payload_header, size);
    if (payload_header != NIL) {
      return payload_vaddr;
    }
  }

  if (last_allocated == FREE) {
    insert_free_block(old_last); // 把刚才取出来的freeblock放回去
  }
  return NIL;
}

#define DEBUG
/*--------------------------------------*/
/*            对外暴露的函数            */
/*--------------------------------------*/
int heap_init() {
  // reset all to 0
  for (int i = 0; i < HEAP_MAX_SIZE; i += 8) {
    *(uint64_t *)&heap[i] = 0;
  }

  heap_start_vaddr = 0;
  heap_end_vaddr = 4096;

  // A epilogue
  // F free block
  // A prologue

  // set the prologue block
  uint64_t prologue_header = get_prologue();
  set_allocated(prologue_header, ALLOCATED);
  set_blocksize(prologue_header, 8);

  uint64_t prologue_footer = prologue_header + 4;
  set_allocated(prologue_footer, ALLOCATED);
  set_blocksize(prologue_footer, 8);

  // set the epilogue block
  // it is a footer only
  uint64_t epilogue = get_epilogue();
  set_allocated(epilogue, ALLOCATED);
  set_blocksize(epilogue, 0);

  // set the block size & allocated of the only regular block
  uint64_t first_header = get_firstblock();
  set_allocated(first_header, FREE);
  set_blocksize(first_header, 4096 - 4 - 8 - 4);

  uint64_t first_footer = get_footer(first_header);
  set_allocated(first_footer, FREE);
  set_blocksize(first_footer, 4096 - 4 - 8 - 4);

  initialize_free_block();

#ifdef DEBUG
  heap_blocks_print();
#endif
  return 1;
}

uint64_t mem_alloc(uint32_t size) {
  assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

  uint32_t alloc_blocksize = 0;
  uint64_t payload_header = search_free_block(size, &alloc_blocksize); // 要size给alloc_blocksize
  uint64_t payload_vaddr = NIL;

  if (payload_header != NIL) {
#ifdef DEBUG
    printf("找到了空闲\n");
#endif
    payload_vaddr = try_alloc_with_splitting(payload_header, alloc_blocksize);
    assert(payload_vaddr != NIL);
  } else {
#ifdef DEBUG
    printf("没找到空闲\n");
#endif
    payload_vaddr = try_extend_heap_to_alloc(alloc_blocksize);
  }
#ifdef DEBUG
    check_heap_correctness();
    check_free_block();
    heap_blocks_print();
#endif
  return payload_vaddr;
}

void mem_free(uint64_t payload_vaddr) {
  if (payload_vaddr == NIL) {
    return;
  }

  // get_firstblock 返回的是 第一个block的header 不是 payload
  assert(get_firstblock() < payload_vaddr && payload_vaddr < get_epilogue());
  assert((payload_vaddr & 0x7) == 0x0);

  // request can be first or last block
  uint64_t req = get_header(payload_vaddr);
  uint64_t req_footer = get_footer(req); // for last block, it is 0

  // get the block_header and allocated of next & prev blocks
  uint64_t next = get_nextheader(req);
  uint64_t prev = get_prevheader(req);
  uint32_t next_allocated = get_allocated(next);
  uint32_t prev_allocated = get_allocated(prev);

  if (next_allocated == ALLOCATED && prev_allocated == ALLOCATED) {
    // case 1 [A|(A->F)|A]
    set_allocated(req, FREE);
    set_allocated(req_footer, FREE);

    insert_free_block(req);
  } else if (next_allocated == FREE && prev_allocated == ALLOCATED) {
    // case 2 [A|(A->F)|F|A] => [A|F+F|A]
    delete_free_block(next);

    uint64_t one_free = merge_blocks_as_free(req, next);

    insert_free_block(one_free);
  } else if (next_allocated == ALLOCATED && prev_allocated == FREE) {
    // case 3 [A|F|(A->F)|A] => [A|F+F|A]
    delete_free_block(prev);

    uint64_t one_free = merge_blocks_as_free(prev, req);

    insert_free_block(one_free);
  } else if (next_allocated == FREE && prev_allocated == FREE) {
    // case 4 [A|F|(A->F)|F|A]
    delete_free_block(prev);
    delete_free_block(next);

    uint64_t one_free = merge_blocks_as_free(merge_blocks_as_free(prev, req), next);

    insert_free_block(one_free);
  } else {
    assert(0);
  }
#ifdef DEBUG
  printf("释放空间\n");
    check_heap_correctness();
    check_free_block();
    heap_blocks_print();
#endif
}
