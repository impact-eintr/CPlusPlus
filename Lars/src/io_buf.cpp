#include <assert.h>
#include <cstdio>
#include <cstring>

#include "io_buf.h"

// 构造一个io_buf对象
io_buf::io_buf(int size) :capacity(size), length(0), head(0), next(nullptr){
  data = new char(size);
  assert(data);
}

// 清空数据
void io_buf::clear() {
  length = head = 0; // 长度为0 且数据头指针位于0
}

// 将处理过的数据清空 将没有处理的数据提前到数据首地址
// 将其他io_buf对象数据拷贝到自己中 只拷贝有效数据
// 处理长度为len的数据 移动head并修正length
