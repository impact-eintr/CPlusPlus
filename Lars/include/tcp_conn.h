#ifndef TCP_CONN_H_
#define TCP_CONN_H_

#include "event_base.h"
#include "net_connection.h"
#include "reactor_buf.h"

// 一个tcp连接
class tcp_conn : public net_connection {
public:
  // 初始化tcp_conn 将当前connfd 注册在loop中
  tcp_conn(int connfd, event_loop *loop);

  // 处理读业务
  void do_read();
  // 处理写业务
  void do_write();
  // 销毁tcp_conn
  void clean_conn();
  // 发送消息的方法
  int send_message(const char *data, int msglen, int msgid);
  // 获取通信文件描述符的接口
  int get_fd() { return _connfd; }

private:
  // 当前连接的fd
  int _connfd;
  // 该连接归属的event_poll
  event_loop *_loop;
  // 输入buf
  input_buf _ibuf;
  // 输出buf
  output_buf _obuf;
};

#endif // TCP_CONN_H_
