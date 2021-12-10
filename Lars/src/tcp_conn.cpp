#include "tcp_conn.h"
#include "event_loop.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <netinet/tcp.h>

// tcp连接的读事件回调函数
static void tcp_conn_read_callback(event_loop* loop, int fd, void* args) {
  tcp_conn* conn = (tcp_conn*)args;
  conn->do_read();
}

// tcp连接的写事件回调函数
static void tcp_conn_write_callback(event_loop* loop, int fd, void* args) {
  tcp_conn* conn = (tcp_conn*)args;
  conn->do_write();
}

// 初始化tcp_conn 将当前connfd 注册在loop中
tcp_conn::tcp_conn(int connfd, event_loop *loop) {
  this->_connfd = connfd;
  this->_loop = loop;

  // 将_connfd设置为非阻塞状态
  int flag = fcntl(_connfd, F_GETFL, 0);

  // 设置TCP_NODELAY禁止做读写缓存 降低小包延迟
  int op = 1;
  setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));

  // 检测用户是否注册了连接建立 hook 有则调用

  // 将该连接的读事件让event_loop监控 一旦有io输入 loop就会有通知
  _loop->add_io_event(_connfd, tcp_conn_read_callback, EPOLLIN, this);

  // 将该连接集成到对应的tcp_server中

}

// 处理读业务
void tcp_conn::do_read() {

}

// 处理写业务
void tcp_conn::do_write() {

}

// 销毁tcp_conn
void tcp_conn::clean_conn() {

}

// 发送消息的方法
int tcp_conn::send_message(const char *data, int msglen, int msgid) {

}
