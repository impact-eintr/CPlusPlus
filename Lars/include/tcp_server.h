#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "event_base.h"
#include <cstdint>
class tcp_server
{
public:
  // tcp_server的构造函数
  tcp_server(event_loop* loop, const char* ip, uint16_t port);

  // 循环创建连接
  void do_accept();

  // 注册消息路由回调函数

  // tcp_server的析构函数
  ~tcp_server();

};

#endif // TCP_SERVER_H_
