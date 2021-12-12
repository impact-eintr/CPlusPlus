#ifndef THREAD_QUEUE_H_
#define THREAD_QUEUE_H_

#include "assert.h"
#include "event_base.h"
#include "event_loop.h"
#include <cstdio>
#include <pthread.h>
#include <queue>
#include <sys/eventfd.h>
#include <unistd.h>

// 每个thread对应的消息任务队列
template <typename T> class thread_queue {
public:
  thread_queue() {
    // eventfd()  creates  an  "eventfd  object"  that can be used as an event
    // wait/notify mechanism by user-space applications, and by the kernel  to
    // notify  user-space  applications of events.  The object contains an un-
    // signed 64-bit integer (uint64_t) counter that is maintained by the ker-
    // nel.  This counter is initialized with the value specified in the argu-
    // ment initval.
    _evfd = eventfd(0, EFD_NONBLOCK);
    if (_evfd == -1) {
      perror("eventfd(0, EFD_NONBLOCK)");
      exit(1);
    }
    _loop = nullptr;
    pthread_mutex_init(&_queue_mutex, nullptr);
  }

  // 向队列添加一个任务 通过send将任务发送给消息队列
  void send(const T &task) {
    // 触发消息事件的占位传输
    unsigned long long idle_num = 1;
    pthread_mutex_lock(&_queue_mutex);

    // 将任务添加到队列 可能是新建连接的任务(包含连接fd) 也可能只是一个普通任务
    _queue.push(task);

    // 向_evfd写 来触发_loop的EPOLLIN事件
    int ret = write(_evfd, &idle_num, sizeof(unsigned long long));
    if (ret == -1) {
      perror("_evfd write");
    }

    pthread_mutex_unlock(&_queue_mutex);
  }

  // 获取队列 当前队列中已经有任务
  // 在io_callback中调用 recv 获取task任务 根据任务的不同类型
  // 处理自定义不同业务流程 请确保new_queue是空队列
  void recv(std::queue<T> &new_queue) {
    assert(new_queue.empty());

    // 触发消息事件的占位传输
    unsigned long long idle_num = 1;
    pthread_mutex_lock(&_queue_mutex);
    // 向_evfd写 来触发_loop的EPOLLOUT事件
    int ret = read(_evfd, &idle_num, sizeof(unsigned long long));
    if (ret == -1) {
      perror("_evfd write");
    }

    // 将当前的队列拷贝出去 将一个空队列换回当前队列 同时清空自身队列
    std::swap(new_queue, _queue);

    pthread_mutex_unlock(&_queue_mutex);
  }

  // 设置当前thread_queue是被那个event_loop监控
  void set_loop(event_loop *loop) { _loop = loop; }

  // 得到监控当前thread_queue的 event_loop
  event_loop *get_loop() { return _loop; }

  // 设置thread_queue的每个业务的回调函数
  void set_callback(io_callback *cb, void *args = nullptr) {
    if (_loop != nullptr) {
      _loop->add_io_event(_evfd, cb, EPOLLIN, args);
    }
  }

private:
  int _evfd;         // 触发消息任务队列读取的每个消息业务的fd
  event_loop *_loop; // 当前消息队列所绑定的event_loop
  std::queue<T> _queue;         // 队列
  pthread_mutex_t _queue_mutex; // 进行添加任务 读取任务的保护锁
};

#endif // THREAD_QUEUE_H_
