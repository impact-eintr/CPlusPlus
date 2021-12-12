#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "event_loop.h"
#include "io_buf.h"
#include "message.h"
#include "net_connection.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

class tcp_client : public net_connection {
public:
  // 初始化客户端套接字
  tcp_client(event_loop *loop, const char *ip, unsigned short port,
             const char *name);

  // 发送message方法
  int send_message(const char *data, int msglen, int msgid);

  // 获取通信文件描述符的接口
  int get_fd() { return _sockfd; }

  // 创建链接
  void do_connect();

  // 处理读业务
  int do_read();

  // 处理写业务
  int do_write();

  // 释放链接资源
  void clean_conn();

  ~tcp_client();

  // 注册消息路由回调函数
  void add_msg_router(int msgid, msg_callback *cb, void *user_data = nullptr) {
    _router.register_msg_router(msgid, cb, user_data);
  }

  //设置链接的创建hook函数
  void set_conn_start(conn_callback cb, void *args = NULL) {
    _conn_start_cb = cb;
    _conn_start_cb_args = args;
  }

  //设置链接的销毁hook函数
  void set_conn_close(conn_callback cb, void *args = NULL) {
    _conn_close_cb = cb;
    _conn_close_cb_args = args;
  }

  //创建链接之后要触发的 回调函数
  conn_callback _conn_start_cb;
  void *_conn_start_cb_args;

  //销毁链接之前要触发的 回调函数
  conn_callback _conn_close_cb;
  void *_conn_close_cb_args;

  bool connected; //链接是否创建成功
  // server端地址
  struct sockaddr_in _server_addr;
  io_buf _ibuf;
  io_buf _obuf;

private:
  int _sockfd;

  socklen_t _addrlen;

  //客户端的事件处理机制
  event_loop *_loop;
  //当前客户端的名称 用户记录日志
  const char *_name;
  // 处理消息的分发路由
  msg_router _router;
};

#endif // TCP_CLIENT_H_
