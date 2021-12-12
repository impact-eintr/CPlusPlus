#ifndef IO_BUF_H_
#define IO_BUF_H_

// buffer 存放数据的结构

class io_buf {
public:
  // 构造 创建一个io_buf对象
  io_buf(int size);

  // 清空数据
  void clear();

  // 已经处理过的数据清空 将没有处理的数据提前到数据首地址
  void adjust();

  // 将其他 io_buf 对象中的数据拷贝给自己
  void copy(const io_buf* other); // 这里小心引用的使用

  // 处理长度为len的数据 移动head并修正length
  void pop(int len);

  // 当前 io_buf 的缓存容量大小
  int capacity;
  // 当前 io_buf 的头部数据长度
  int length;
  // 未处理数据的头部位置索引
  int head;
  // 如果存在多个io_buf 使用链表的形式连接起来
  io_buf *next;
  // 当前io_buf保存数据的数据首地址
  char *data;
};

#endif // IO_BUF_H_
