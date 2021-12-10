#ifndef TASK_MSG_H_
#define TASK_MSG_H_

#include "event_loop.h"

// 定义异步任务回调函数类型
// arg1 : 使用哪一个loop机制去执行这个task任务 每个loop机制对应着一个thread
// arg2 : 任务的参数
typedef void (*task_func)(event_loop *loop, void *args);

// 一个任务的抽象
struct task_msg {
  enum TASK_TYPE {
    NEW_CONN, // 新建连接的任务
    NEW_TASK, // 一般的任务
  };

  TASK_TYPE type;

  // 任务的一些参数
  union {
    // 针对 NEW_TASK 新建连接任务 需要创建connfd
    int connfd;

    // 针对 NEW_TASK 新建任务 这里提供一个回调函数
    struct {
      void (*task_cb)(event_loop *, void *args);
      void *args;
    };
  };
};

#endif // TASK_MSG_H_
