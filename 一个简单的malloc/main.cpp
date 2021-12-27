#include "include/allocator.h"
#include <cstdint>
#include <iostream>
using namespace std;

int main() {
  cout << "hello world" << endl;
  heap_init();

  int * arr1 = (int *)mem_alloc(sizeof(int) * 8);

  int * arr2 = (int *)mem_alloc(sizeof(int) * 4);

  int * arr3 = (int *)mem_alloc(sizeof(int) * 128);

  int * arr4 = (int *)mem_alloc(sizeof(int) * 8);

  int * arr5 = (int *)mem_alloc(sizeof(int) * 4);

  mem_free(uint64_t(arr2));
  mem_free(uint64_t(arr4));

  mem_free(uint64_t(arr3));


  //mem_free(uint64_t(arr1));
  //mem_free(uint64_t(arr5));


}
