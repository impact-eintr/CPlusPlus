#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


#include "../headers/algorithm.h"

int main() {
  hashtable_t *tab = hashtable_construct(8);
  for (int i = 0;i < 100; ++i) {
    char buf[8];
    sprintf(buf, "key%d", i);
    hashtable_insert(tab, buf, i);
  }

  uint64_t res = 0;
  hashtable_get(tab, "key0", &res);
  printf("0x%lx\n", res);
  hashtable_get(tab, "tkey1", &res);
  printf("0x%lx\n", res);
  hashtable_get(tab, "key2", &res);
  printf("0x%lx\n", res);
  hashtable_get(tab, "key3", &res);
  printf("0x%lx\n", res);

  print_hashtable(tab);

  hashtable_free(tab);
}
