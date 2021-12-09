#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#define MAXEVENTS 10

#include "event_base.h"
#include <ext/hash_map>
#include <ext/hash_set>
#include <sys/epoll.h>
#include <vector>
#include <utility>

// 定义异步任务回调函数类型
typedef void (*task_func)(event_loop *loop, void *args);

// map: fd->io_event
typedef __gnu_cxx::hash_map<int, io_event> io_event_map;

// 定义指向上面map类型的迭代器
typedef __gnu_cxx::hash_map<int, io_event>::iterator io_event_map_it;

// 全部正在监听的fd集合
typedef __gnu_cxx::hash_set<int> listen_fd_set;

// event_loop事件处理机制
class event_loop {
public:
  // 构造 初始化epoll堆
  event_loop();

  // 阻塞循环处理事件
  void event_process();

  // 添加一个io事件到loop中
  void add_io_event(int fd, io_callback *proc, int mask, void *args = nullptr);

  // 删除一个io事件从loop中
  void del_io_event(int fd);

  // 删除一个io事件的EPOLLIN/EPOLLOUT
  void del_io_event(int fd, int mask);

  // 获取全部监听事件的fd集合
  void get_listen_fds(listen_fd_set &fds) {
    fds = listen_fds;
  }

  // 添加一个任务task到ready_task中
  void add_task(task_func func, void *args);

  // 执行全部的ready_task中的任务
  void execute_ready_tasks();

private:
  // epoll fd
  int _epfd;

  // 是一个hash_map对象，主要是方便我们管理fd<—>io_event的对应关系，方便我们来查找和处理。
  io_event_map _io_evs;

  // 记录目前一共有多少个fd正在本我们的event_loop机制所监控
  listen_fd_set listen_fds;

  // 已经通过epoll_wait返回的被激活需要上层处理的fd集合
  struct epoll_event _fired_eves[MAXEVENTS];

  // 需要被执行的task集合
  typedef std::pair<task_func, void*> task_func_pair; // 回调函数与参数的键值对
  std::vector<task_func_pair> _ready_tasks; // 所有已经就待执行的任务集合
};

#endif // EVENT_LOOP_H_
