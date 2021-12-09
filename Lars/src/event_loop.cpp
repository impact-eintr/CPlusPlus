#include "event_loop.h"
#include "event_base.h"
#include "task_msg.h"

#include <assert.h>
#include <sys/epoll.h>
#include <vector>

// 构造 初始化epoll堆
event_loop::event_loop() {
  // flag=0等价于epoll_create
  // _epfd是一个文件描述符 代表着当前的event_loop与操作系统申请的epoll资源
  this->_epfd = epoll_create1(0);
  if (this->_epfd == -1) {
    fprintf(stderr, "epoll_create error\n");
    exit(1);
  }
}

// 阻塞循环处理事件
void event_loop::event_process() {
  while (true) {
    io_event_map_it ev_it;

    // The epoll_wait() system call waits for events on the epoll(7)  instance
    // referred  to  by  the  file  descriptor epfd.  The buffer pointed to by
    // events is used to return information from the ready list about file de‐
    // scriptors  in the interest list that have some events available.  Up to
    // maxevents are returned by epoll_wait().  The maxevents argument must be
    // greater than zero.
    int nfds = epoll_wait(this->_epfd, _fired_eves, MAXEVENTS, 10);
    for (int i = 0; i < nfds; i++) {
      // 通过触发的fd找到对应的绑定事件
      // _ep_fd -> events[] -取出-> events[i].data.fd
      // -通过这个key找到保存下来的io_event-> io_event
      ev_it = _io_evs.find(_fired_eves[i].data.fd);
      assert(ev_it != _io_evs.end());

      // 这是一个 struct io_event 是一个抽象的io事件 包含有 mask
      // 读/写回调函数及参数 之后会根据当前发生的事件 操作io_event
      io_event *ev = &(ev_it->second);

      if (_fired_eves[i].events & EPOLLIN) {
        // epoll 读事件 调用io_event中注册好的回调函数
        void *args = ev->rcb_args;
        ev->read_callback(this, _fired_eves[i].data.fd, args);
      } else if (_fired_eves[i].events & EPOLLOUT) {
        // epoll 写事件 调用io_event中注册好的回调函数
        void *args = ev->wcb_args;
        ev->write_callback(this, _fired_eves[i].data.fd, args);
      } else if (_fired_eves[i].events & (EPOLLHUP | EPOLLERR)) {
        // 水平触发未处理 可能出现HUP事件 这种正常处理读写就好 要是没有直接清空
        if (ev->read_callback != nullptr) {
          void *args = ev->rcb_args;
          ev->read_callback(this, _fired_eves[i].data.fd, args);
        } else if (ev->write_callback != nullptr) {
          void *args = ev->wcb_args;
          ev->write_callback(this, _fired_eves[i].data.fd, args);
        } else {
          // 删除
          fprintf(stderr, "fd %d get error. delete it from epoll\n",
                  _fired_eves[i].data.fd);
          this->del_io_event(_fired_eves[i].data.fd);
        }
      }
    }
    // 每次处理完一组epoll_wait触发的事件后 处理异步任务
    this->execute_ready_tasks();
  }
}

// 注册事件
/*
 * 这里我们处理的事件机制是
 * 如果EPOLLIN 在mask中， EPOLLOUT就不允许在mask中
 * 如果EPOLLOUT 在mask中， EPOLLIN就不允许在mask中
 * 如果想注册EPOLLIN|EPOLLOUT的事件， 那么就调用add_io_event() 方法两次来注册。
 * */
void event_loop::add_io_event(int fd, io_callback *proc, int mask, void *args) {
  int final_mask;
  int op;

  // 找到当前fd是否已经有事件
  io_event_map_it it = _io_evs.find(fd);
  if (it == _io_evs.end()) {
    // 没找到这个事件 添加进去
    final_mask = it->second.mask | mask;
    op = EPOLL_CTL_ADD;
  } else {
    // 找到相关事件 修改MASK位
    final_mask = it->second.mask | mask;
    op = EPOLL_CTL_MOD;
  }

  // 保存好MASK 和 操作指令后 开始注册这个事件(fd +
  // io_callback)到event_loop机制中

  // 1 注册回调函数
  if (mask & EPOLLIN) {
    // 读事件回调函数注册
    _io_evs[fd].read_callback = proc;
    _io_evs[fd].rcb_args = args;
  } else if (mask & EPOLLOUT) {
    // 写事件回调函数注册
    _io_evs[fd].write_callback = proc;
    _io_evs[fd].wcb_args = args;
  }

  // 2 epoll_ctl 添加到 _epfd 代表的epoll堆中
  _io_evs[fd].mask = final_mask;
  // 创建原生epoll事件
  struct epoll_event event;
  event.events = final_mask;
  event.data.fd = fd;
  if (epoll_ctl(_epfd, op, fd, &event) == -1) {
    fprintf(stderr, "epoll ctl %d error\n", fd);
    return;
  }

  // 3 把这个fd 添加到监听集合中
  listen_fds.insert(fd);
}

// 删除事件
void event_loop::del_io_event(int fd) {
  // 将事件从_io_evs删除
  _io_evs.erase(fd);

  // 将fd从监听集合中删除
  listen_fds.erase(fd);

  // 将fd 从epoll堆中删除
  epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

// 修正事件
void event_loop::del_io_event(int fd, int mask) {
  // 如果没有该事件 直接返回
  io_event_map_it it = _io_evs.find(fd);
  if (it == _io_evs.end()) {
    return;
  }

  int &o_mask = it->second.mask;
  // 修正mask
  o_mask = o_mask & (~mask);

  if (o_mask == 0) {
    // 如果修正之后 mask为0 则删除
    this->del_io_event(fd);
  } else {
    // 如果修正后 mask非0 则修改
    struct epoll_event event;
    event.events = o_mask;
    event.data.fd = fd;
    epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event);
  }
}

// 添加一个任务task到read_tasks集合中
void event_loop::add_task(task_func func, void *args) {
  task_func_pair func_pair(func, args);
  _ready_tasks.push_back(func_pair);
}

// 执行全部的ready_tasks中的任务
void event_loop::execute_ready_tasks() {
  std::vector<task_func_pair>::iterator it;

  for (it = _ready_tasks.begin(); it != _ready_tasks.end(); it++) {
    task_func func = it->first; //任务回调函数
    void *args = it->second;    //回调函数形参

    //执行任务
    func(this, args);
  }

  //全部执行完毕，清空当前的_ready_tasks
  _ready_tasks.clear();
}
