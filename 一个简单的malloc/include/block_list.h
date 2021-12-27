#ifndef SMALL_LIST_H_
#define SMALL_LIST_H_

#include <cstdint>

void block_list_init();
void block_list_insert(uint64_t free_header);
void block_list_delete(uint64_t free_header);
void block_list_check_free_blocks();

#endif // SMALL_LIST_H_
