#include <assert.h>
#include <cstdio>
#include <cstring>

#include "io_buf.h"

// 构造一个io_buf对象
io_buf::io_buf(int size) : capacity(size), length(0), head(0), next(nullptr) {
  data = new char[size];
  assert(data);
}

// 清空数据
void io_buf::clear() {
  length = head = 0; // 长度为0 且数据头指针位于0
}

// 将处理过的数据清空 将没有处理的数据提前到数据首地址
void io_buf::adjust() {
  if (head != 0) {
#ifdef debug
  printf("[adjust]%d %d \n", head, length);
#endif
    if (length != 0) {
      memmove(data, data + head, length);
    }
    head = 0;
  }
}

// 将其他io_buf对象数据拷贝到自己中 只拷贝有效数据
void io_buf::copy(const io_buf *other) {
  memcpy(data, other->data + other->head, other->length);
  head = 0;
  length = other->length;
}

// 处理长度为len的数据 移动head并修正length
void io_buf::pop(int len) {
  length -= len;
  head += len;
#ifdef debug
  printf("[pop]%d %d \n", head, length);
#endif
}
