#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "event_loop.h"
#include "message.h"
#include "net_connection.h"
#include "tcp_conn.h"
#include "thread_pool.h"

#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>
class tcp_server {
public:
  // tcp_server的构造函数
  tcp_server(event_loop *loop, const char *ip, uint16_t port);

  // tcp_server的析构函数
  ~tcp_server();

  // 循环创建连接
  void do_accept();

  // 注册消息路由回调函数
  void add_msg_router(int msgid, msg_callback *cb, void *user_data = nullptr) {
    router.register_msg_router(msgid, cb, user_data);
  }

  // 获取当前Server的线程池
  thread_pool *get_thread_pool() { return _thread_pool; }

private:
  // 基础信息
  int _socketfd;                // 创建监听端口的socket的fd
  struct sockaddr_in _connaddr; // 客户端连接地址
  socklen_t _addrlen;           // 客户端连接地址长度
  event_loop *_loop;            // server的event_loop机制

  // 连接相关
  static int _max_conns;               // 最大连接限制
  static int _curr_conns;              // 当前连接统计
  static pthread_mutex_t _conns_mutex; // 保护_curr_conns
  thread_pool *_thread_pool; // 每个server拥有一个可调度的thread_pool

public:
  // 客户端连接管理部分
  static void increase_conn(int connfd, tcp_conn *conn); // 新建tcp_conn
  static void decrease_conn(int connfd);    // 减少一个断开的连接
  static void get_conn_num(int *curr_conn); // 获取当前连接数
  static tcp_conn **conns; // 全部已经连接好的连接信息 可以认为是一个数组

  // 消息分发路由
  static msg_router router;

  // 创建连接之后要触发的 回调函数
  static conn_callback conn_start_cb;
  static void *conn_start_cb_args;

  // 销毁连接之后要触发的 回调函数
  static conn_callback conn_close_cb;
  static void *conn_close_cb_args;

  // 创建 / 销毁 连接之后要触发的 Hook函数
  static void set_conn_start(conn_callback cb, void *args = nullptr) {
    conn_start_cb = cb;
    conn_start_cb_args = args;
  }

  static void set_conn_close(conn_callback cb, void *args = nullptr) {
    conn_close_cb = cb;
    conn_close_cb_args = args;
  }
};

#endif // TCP_SERVER_H_
