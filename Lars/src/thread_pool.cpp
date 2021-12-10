#include "thread_pool.h"
#include "event_loop.h"
#include "task_msg.h"
#include "tcp_conn.h"

#include <cstdio>
#include <pthread.h>
#include <unistd.h>

// 这个函数是用来处理消息队列中消息的
// 只要有人调用 thread_queue::send() 方法就会触发
// thread_pool() --> thread_cnt个线程 --> 每个线程执行thread_main(queue) -
// -> 开启event_loop机制 监控queue中发生的io事件 也就是queue->_evfd的事件
void deal_task_message(event_loop *loop, int fd, void *args) {
  // 获取触发事件的消息队列即thread_queue
  thread_queue<task_msg> *queue = (thread_queue<task_msg> *)args;

  // 将queu中的全部任务取出来
  std::queue<task_msg> tasks;
  queue->recv(tasks);

  while (tasks.empty() != true) {
    task_msg task = tasks.front();

    // 取出的task出队
    tasks.pop();

    if (task.type == task_msg::NEW_CONN) {
      // 这是一个新建连接的任务  把这个tcp_conn加入到当前线程的loop中去监听
      tcp_conn *conn = new tcp_conn(task.connfd, loop);
      if (conn == nullptr) {
        fprintf(stderr, "in thread new tcp_conn error\n");
        exit(1);
      }
      printf("[thread]: get new connection succ!\n");
    } else if (task.type == task_msg::NEW_TASK) {
      // 一个普通的任务
      loop->add_task(task.task_cb, task.args);
    } else {
      fprintf(stderr, "unknow task!\n");
    }
  }
}

// 每个线程的主业务
void *thread_main(void *args) {
  thread_queue<task_msg> *queue = (thread_queue<task_msg> *)args;

  // 每个线程开启自己单独的event_loop机制
  event_loop *loop = new event_loop();
  if (loop == nullptr) {
    fprintf(stderr, "new event_loop error\n");
    exit(1);
  }
  // 注册一个触发消息任务的callback函数
  queue->set_loop(loop);
  queue->set_callback(deal_task_message, queue);

  // 启动阻塞监听
  loop->event_process();

  return nullptr;
}

thread_pool::thread_pool(int thread_cnt)
    : _queues(nullptr), _thread_cnt(thread_cnt), _index(0) {
  if (_thread_cnt <= 0) {
    fprintf(stderr, "_thread_cnt <= 0\n");
    exit(1);
  }

  // 任务队列的个数和线程个数一致
  _queues = new thread_queue<task_msg> *[thread_cnt];
  _tids = new pthread_t[thread_cnt];

  int ret;
  for (int i = 0; i < thread_cnt; ++i) {
    // 创建一个线程
#ifdef debug
    printf("create %d thread\n", i);
#endif
    // 给当前线程创建一个任务队列
    _queues[i] = new thread_queue<task_msg>();
    ret = pthread_create(&_tids[i], nullptr, thread_main, _queues[i]);
    if (ret == -1) {
      perror("thread_pool, create thread");
      exit(1);
    }
    // 将线程脱离
    pthread_detach(_tids[i]);
  }
}

thread_queue<task_msg> *thread_pool::get_thread() {
  return _queues[_index % _thread_cnt];
}

void thread_pool::send_task(task_func func, void *args) {
  task_msg task;
  // 给当前thread_pool中的每一个thread中的pool添加一个task任务
  for (int i = 0; i < _thread_cnt; ++i) {
    // 封装一个task消息
    task.type = task_msg::NEW_TASK;
    task.task_cb = func;
    task.args = args;
    // 取出第i个thread的消息队列
    thread_queue<task_msg> *queue = _queues[i];
    // 发送task消息
    queue->send(task);
  }
}
