#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../headers/algorithm.h"

#define TIMES (10000000)

int main() {
  hashtable_t *tab = hashtable_construct();
  for (int i = 0; i < TIMES; ++i) {
    char buf[1024];
    sprintf(buf, "key%d", i);
    hashtable_insert(tab, buf, i + 100);
  }

  for (int i = 0; i < TIMES; ++i) {
    uint64_t res = 0;
    char buf[8];
    sprintf(buf, "key%d", i);
    hashtable_get(tab, buf, &res);
    if (res != 0) {
      printf("%s : %ld\n", buf, res);
    } else {
      printf("没找到 %s\n", buf);
    }
  }
  hashtable_free(tab);
}
