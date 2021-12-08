#include "reactor_buf.h"
#include "buf_pool.h"

#include <assert.h>
#include <cstring>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

// ==================== reactor_buf ============================

reactor_buf::reactor_buf() { _buf = nullptr; }
reactor_buf::~reactor_buf() { clear(); }

// 只有类的非静态成员函数后可以加const修饰，表示该类的this指针为const类型，不能改变类的成员变量的值
const int reactor_buf::length() const {
  return _buf != nullptr ? _buf->length : 0;
}

void reactor_buf::pop(int len) {
  assert(_buf != nullptr && len <= _buf->length);

  _buf->pop(len);
  // 如果这是的_buf的可用长度已经为0
  if (_buf->length == 0) {
    // 将_buf重新放回buf_pool中
    buf_pool::instance().revert(_buf);
    _buf = nullptr;
  }
}

void reactor_buf::clear() {
  if (_buf != nullptr) {
    // 将_buf重新放回buf_pool中
    buf_pool::instance().revert(_buf);
    _buf = nullptr;
  }
}

// ==================== input_buf ============================
int input_buf::read_data(int fd) {
  int need_read; // 硬件有多少数据是可以读取的
  // 一次性读出所有的数据
  // 需要给fd设置FIONREAD
  // 得到read缓冲中有多少数据是可以读取的
  if (ioctl(fd, FIONREAD, &need_read) == -1) {
    fprintf(stderr, "ioctl FIONREAD\n");
    return -1;
  }

  if (_buf == nullptr) {
    // 如果io_buf为空 从内存池申请
    _buf = buf_pool::instance().alloc_buf(need_read);
    if (_buf == nullptr) {
      fprintf(stderr, "no idle buf for alloc\n");
      return -1;
    }
  } else {
    // 如果io_buf可用 判断是否够用
    assert(_buf->head == 0);
    // 容量-有用的数据=空闲空间 <=== 需要读取的数据
    if (_buf->capacity - _buf->length < (int)need_read) {
      // 不够存
      io_buf *new_buf =
          buf_pool::instance().alloc_buf(need_read + _buf->length);

      if (new_buf == nullptr) {
        fprintf(stderr, "no ilde buf for alloc\n");
        return -1;
      }

      // 将之前_buff中的数据拷贝到新申请的buf中
      new_buf->copy(_buf);
      // 将之前的_buf放回到内存池中
      buf_pool::instance().revert(_buf);
      // 新申请的buf成为当前的io_buf
      _buf = new_buf;
    }
  }
  // 读取数据
  int alread_read = 0;
  do {
    // 读取的数据拼接到之前的数据后
    if (need_read == 0) {
      // 可能是read阻塞读数据的模式 对方未写入数据
      alread_read = read(fd, _buf->data + _buf->length, m4K);
    } else {
      alread_read = read(fd, _buf->data + _buf->length, need_read);
    }
  } while (
      alread_read == -1 &&
      errno ==
          EINTR); // systemCall引起的中断 继续读取 防止可重入导致读取中途失败

  if (alread_read > 0) {
    if (need_read != 0) {
      assert(alread_read == need_read);
    }
    _buf->length += alread_read;
  }
  return alread_read;
}

// 取出读到的数据的首地址
const char *input_buf::data() const {
  return _buf != nullptr ? _buf->data + _buf->head : nullptr;
}

// 重置缓冲区
void input_buf::adjust() {
  if (_buf != nullptr) {
    _buf->adjust();
  }
}

// ==================== output_buf ============================
int output_buf::write2buf(const char *data, int datalen) {
  if (_buf == nullptr) {
    // 如果io_buf为空 从内存池中申请一块io_buf
    _buf = buf_pool::instance().alloc_buf(datalen);
    if (_buf == nullptr) {
      fprintf(stderr, "no idle buf for alloc\n");
      return -1;
    }
  } else {
    assert(_buf->head == 0);
    if (_buf->capacity - _buf->length < datalen) {
      // 不够使用 重新申请
      io_buf *new_buf = buf_pool::instance().alloc_buf(datalen + _buf->length);
      if (new_buf == nullptr) {
        fprintf(stderr, "no idle buf for alloc\n");
        return -1;
      }
      // 将之前的_buf的数据拷贝到新申请的new_buf中
      new_buf->copy(_buf); // 注意发生在这里的拷贝
      buf_pool::instance().revert(_buf);
      // 替换buf
      _buf = new_buf;
    }
  }
  // 将data数据拷贝到io_buf中 拼接到后面
  memcpy(_buf->data + _buf->length, data, datalen);
  _buf->length += datalen;
  return 0;
}

int output_buf::write2fd(int fd) {
  assert(_buf != nullptr && _buf->head == 0);

  int alread_write = 0;

  do {
    alread_write = write(fd, _buf->data, _buf->length);
  } while (alread_write == -1 && errno == EINTR);

  if (alread_write > 0) {
    // 将已经写过的数据清空
    _buf->pop(alread_write);
    // 将未处理数据前置覆盖老数据
    _buf->adjust();
  }
  // 如果fd非阻塞 可能得到EAGAIN错误
  if (alread_write == -1 && errno == EAGAIN) {
    alread_write = 0; // 不是错误 仅仅返回0 表示当前暂时不可以继续写
  }
  return alread_write;
}
