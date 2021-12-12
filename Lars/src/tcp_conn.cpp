#include "tcp_conn.h"
#include "buf_pool.h"
#include "event_loop.h"
#include "message.h"
#include "tcp_server.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// tcp连接的读事件回调函数
static void tcp_conn_read_callback(event_loop *loop, int fd, void *args) {
  tcp_conn *conn = (tcp_conn *)args;
  conn->do_read();
}

// tcp连接的写事件回调函数
static void tcp_conn_write_callback(event_loop *loop, int fd, void *args) {
  tcp_conn *conn = (tcp_conn *)args;
  conn->do_write();
}

// 初始化tcp_conn 将当前connfd 注册在loop中
// _loop就是当前线程生成时 构建的那个event_loop
// 每有一个新的连接到来 就会生成一个thread
tcp_conn::tcp_conn(int connfd, event_loop *loop) {
  this->_connfd = connfd;
  this->_loop = loop;

  // 将_connfd设置为非阻塞状态
  int flag = fcntl(_connfd, F_GETFL, 0);
  fcntl(_connfd, F_SETFL, flag | O_NONBLOCK);

  // 设置TCP_NODELAY禁止做读写缓存 降低小包延迟
  int op = 1;
  setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));

  // 检测用户是否注册了连接建立 hook 有则调用
  if (tcp_server::conn_start_cb) {
    tcp_server::conn_start_cb(this, tcp_server::conn_start_cb_args);
  }
  // 将该连接的读事件让event_loop监控 一旦有io输入 loop就会有通知
  _loop->add_io_event(_connfd, tcp_conn_read_callback, EPOLLIN, this);

  // 将该连接集成到对应的tcp_server中
  tcp_server::increase_conn(_connfd, this);
}

// 处理读业务
void tcp_conn::do_read() {
  // 从套接字读取数据
  int ret = _ibuf.read_data(_connfd);
  if (ret == -1) {
    fprintf(stderr, "read data from socket\n");
    this->clean_conn();
    return;
  } else if (ret == 0) {
    // 对端正常关闭
    printf("connection closed by peer\n");
    clean_conn();
    return;
  }

  // 处理读到的数据
  msg_head head;

  while (_ibuf.length() >= MESSAGE_HEAD_LEN) {
    // 读取msg_head头部 固定长度MESSAGE_HEAD_LEN
    memcpy(&head, _ibuf.data(), MESSAGE_HEAD_LEN);
    if (head.msglen > MESSAGE_MAX_LEN || head.msglen < 0) {
      fprintf(stderr, "data format error, need close, msglen=%d\n",
              head.msglen);
      this->clean_conn();
      break;
    }

    // 检测_ibuf中的数据包是否是完整的
    if (_ibuf.length() < MESSAGE_HEAD_LEN + head.msglen) {
      break;
    }
    _ibuf.pop(MESSAGE_HEAD_LEN);

#ifdef debug
    printf("read data: %s\n", _ibuf.data());
#endif
    // 消息包按照 msgid 进行路由处理 分发给不同的处理逻辑
    tcp_server::router.call_msg_router(head.msgid, head.msglen, _ibuf.data(),this);

    // 消息处理完毕
    _ibuf.pop(head.msglen);
  }
  _ibuf.adjust();
  return;
}

// 处理写业务
void tcp_conn::do_write() {
  while (_obuf.length()) {
    int ret = _obuf.write2fd(_connfd);
    if (ret == -1) {
      fprintf(stderr, "write2fd error, close conn\n");
      this->clean_conn();
      return;
    }
    if (ret == 0) {
      // 不是错误 表示不可继续写
      break;
    }
  }
#ifdef debug
  printf("server do_write() over!");
#endif
  if (_obuf.length() == 0) {
    // 数据全部写完 将_connfd的写事件取消掉
    _loop->del_io_event(_connfd, EPOLLOUT);
  }
  return;
}

// 销毁tcp_conn
void tcp_conn::clean_conn() {
  // 如果注册了销毁Hook函数 则调用
  if (tcp_server::conn_close_cb) {
    tcp_server::conn_close_cb(this, tcp_server::conn_close_cb_args);
  }

  // 连接清理工作
  tcp_server::decrease_conn(_connfd);

  _loop->del_io_event(_connfd);

  // buf清空
  _ibuf.clear();
  _obuf.clear();

  // 关闭原始套接字
  int fd = _connfd;
  _connfd = -1;
  close(fd);
}

// 发送消息的方法
int tcp_conn::send_message(const char *data, int msglen, int msgid) {
#define debug
#ifdef debug
  printf("server send_message:%s.%d, msgid=%d\n", data, msglen, msgid);
#endif
  bool active_epollout = false;
  if (_obuf.length() == 0) {
    // 如果现在数据都已经发送完了 那么是一定要激活写事件 的
    // 如果有数据 说明数据还没有完全写到对端 那么没必要再激活 等写完再激活
    active_epollout = true;
  }

  // 先封装message消息头
  msg_head head;
  head.msglen = msglen;
  head.msgid = msgid;

  // 写消息头
  int ret = _obuf.write2buf((const char *)&head, MESSAGE_HEAD_LEN);
  if (ret != 0) {
    fprintf(stderr, "send head error\n");
    return 1;
  }

  // 写消息体
  ret = _obuf.write2buf(data, msglen);
  if (ret != 0) {
    // 如果写消息失败 那就回滚 将消息头的发送也取消
    _obuf.pop(MESSAGE_HEAD_LEN);
    return -1;
  }

  if (active_epollout == true) {
    // 激活EPOLLOUT事件
    _loop->add_io_event(_connfd, tcp_conn_write_callback, EPOLLOUT, this);
  }

  return 0;
}
