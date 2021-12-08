#ifndef EVENT_BASE_H_
#define EVENT_BASE_H_

// 定义一些IO复用机制或其他一擦还能给触发机制的事件封装

class event_loop;

typedef void io_callback(event_loop &loop, int fd, void *args);

// 封装IO触发实现
struct io_event {
  io_event()
      : read_callback(nullptr), write_callback(nullptr), rcb_args(nullptr),
        wcb_args(nullptr) {}
  int mask;
  io_callback *read_callback;
  io_callback *write_callback;
  void *rcb_args;
  void *wcb_args;
};

#endif // EVENT_BASE_H_
