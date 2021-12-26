#include <cstdint>
#include <cassert>
#include "algorithm.h"
#include "allocator.h"

#include "block_list.h"

linkedlist_internal_t block_list;

/* ------------------------------------- */
/*  Operations for List Block Structure  */
/* ------------------------------------- */

static int compare_nodes(uint64_t first, uint64_t second) {
  return !(first == second);
}

static int is_null_node(uint64_t node_id) { return node_id == NULL_ID; }

static uint64_t get_prevfree_block8(uint64_t header_vaddr) {
  assert(header_vaddr % 8 == 4);
  assert(get_allocated(header_vaddr) == FREE);
  // header is 8X + 4
  uint32_t value = *(uint32_t *)&heap[header_vaddr];
  return 4 + (value & 0xFFFFFFF8);
}

static uint64_t get_nextfree_block8(uint64_t header_vaddr) {
  assert(header_vaddr % 8 == 4);
  assert(get_allocated(header_vaddr) == FREE);
  // header is 8X + 4
  uint32_t value = *(uint32_t *)&heap[header_vaddr + 4];
  return 4 + (value & 0xFFFFFFF8);
}

static int set_prevfree_block8(uint64_t header_vaddr, uint64_t prev_vaddr) {
  // we set by header only
  assert(header_vaddr % 8 == 4);
  assert(get_allocated(header_vaddr) == FREE);
  assert(prev_vaddr % 8 == 4);
  *(uint32_t *)&heap[header_vaddr] &= 0x00000007; // reset prev pointer
  *(uint32_t *)&heap[header_vaddr] |= (prev_vaddr & 0xFFFFFFF8);
  return 1;
}

static int set_nextfree_block8(uint64_t header_vaddr, uint64_t next_vaddr) {
  // we set by header only
  assert(header_vaddr % 8 == 4);
  assert(get_allocated(header_vaddr) == FREE);
  assert(next_vaddr % 8 == 4);
  header_vaddr += 4;
  *(uint32_t *)&heap[header_vaddr] &= 0x00000007; // reset next pointer
  *(uint32_t *)&heap[header_vaddr] |= (next_vaddr & 0xFFFFFFF8);
  return 1;
}

// register the 5 functions above to be called by the linked list framework
static linkedlist_node_interface i_block8 = {
    .is_null_node = &is_null_node,
    .compare_nodes = &compare_nodes,
    .get_node_prev = &get_prevfree_block8,
    .set_node_prev = &set_prevfree_block8,
    .get_node_next = &get_nextfree_block8,
    .set_node_next = &set_nextfree_block8,
};

/* ------------------------------------- */
/*  Operations for Linked List           */
/* ------------------------------------- */

static int update_head(linkedlist_internal_t *this_l, uint64_t block_vaddr) {
  if (this_l == nullptr) {
    return 0;
  }

  assert(block_vaddr == NULL_ID ||
         (get_firstblock() <= block_vaddr && block_vaddr <= get_lastblock()));
  assert(block_vaddr == NULL_ID || block_vaddr % 8 == 4);
  assert(block_vaddr == NULL_ID || get_blocksize(block_vaddr) == 8);

  this_l->head = block_vaddr;
  return 1;
}

void block_list_init() {
  block_list.head = NULL_ID;
  block_list.count = 0;
  block_list.update_head = &update_head;
}

void block_list_insert(uint64_t free_header) {
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(free_header % 8 == 4);
  assert(get_blocksize(free_header) == 8);
  assert(get_allocated(free_header) == FREE);

  linkedlist_internal_insert(&block_list, &i_block8, free_header);
}

void block_list_delete(uint64_t free_header) {
  assert(get_firstblock() <= free_header && free_header <= get_lastblock());
  assert(free_header % 8 == 4);
  assert(get_blocksize(free_header) == 8);

  linkedlist_internal_delete(&block_list, &i_block8, free_header);
}

void check_size_list_correctness(linkedlist_internal_t *list,
                                 linkedlist_node_interface *i_node,
                                 uint32_t min_size, uint32_t max_size) {
  uint32_t counter = 0;
  uint64_t b = get_firstblock();
  int head_exists = 0;
  while (b <= get_lastblock()) {
    uint32_t bsize = get_blocksize(b);
    if (get_allocated(b) == FREE && min_size <= bsize && bsize <= max_size) {
      uint64_t prev = i_node->get_node_prev(b);
      uint64_t next = i_node->get_node_next(b);
      uint64_t prev_next = i_node->get_node_next(prev);
      uint64_t next_prev = i_node->get_node_prev(next);

      assert(get_allocated(prev) == FREE);
      assert(get_allocated(next) == FREE);
      assert(prev_next == b);
      assert(next_prev == b);

      if (b == list->head) {
        head_exists = 1;
      }

      counter += 1;
    }
    b = get_nextheader(b);
  }
  assert(list->count == 0 || head_exists == 1);
  assert(counter == list->count);

  uint64_t p = list->head;
  uint64_t n = list->head;
  for (int i = 0; i < list->count; ++i) {
    uint32_t psize = get_blocksize(p);
    uint32_t nsize = get_blocksize(n);

    assert(get_allocated(p) == FREE);
    assert(min_size <= psize && psize <= max_size);

    assert(get_allocated(n) == FREE);
    assert(min_size <= nsize && nsize <= max_size);

    p = i_node->get_node_prev(p);
    n = i_node->get_node_next(n);
  }
  assert(p == list->head);
  assert(n == list->head);
}

void block_list_check_free_blocks() {
  check_size_list_correctness(&block_list, &i_block8, 8, 8);
}
