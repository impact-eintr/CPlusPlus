#ifndef SMALL_LIST_H_
#define SMALL_LIST_H_

#include <cstdint>

void small_list_init();
void small_list_insert(uint64_t free_header);
void small_list_delete(uint64_t free_header);
void small_list_check_free_blocks();

#endif // SMALL_LIST_H_
