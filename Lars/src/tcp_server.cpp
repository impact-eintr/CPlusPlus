#include "tcp_server.h"
#include "config_file.h"
#include "net_connection.h"
#include "reactor_buf.h"
#include "tcp_conn.h"

#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>
#include <strings.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <unistd.h>

tcp_conn** tcp_server::conns = nullptr;
int tcp_server::_max_conns = 10; // 最大连接限制
int tcp_server::_curr_conns = 0; // 当前连接统计
pthread_mutex_t tcp_server::_conns_mutex =
    PTHREAD_MUTEX_INITIALIZER; // 保护_curr_conns

// 客户端连接管理部分
// 新建tcp_conn
void tcp_server::increase_conn(int connfd, tcp_conn *conn) {
  pthread_mutex_lock(&tcp_server::_conns_mutex);
  tcp_server::conns[connfd] = conn;
  tcp_server::_curr_conns++;
  pthread_mutex_unlock(&tcp_server::_conns_mutex);
}

// 减少一个断开的连接
void tcp_server::decrease_conn(int connfd) {
  pthread_mutex_lock(&tcp_server::_conns_mutex);
  tcp_server::conns[connfd] = nullptr;
  tcp_server::_curr_conns--;
  pthread_mutex_unlock(&tcp_server::_conns_mutex);
}

// 获取当前连接数
void tcp_server::get_conn_num(int *curr_conn) {
  pthread_mutex_lock(&tcp_server::_conns_mutex);
  *curr_conn = tcp_server::_curr_conns;
  pthread_mutex_unlock(&tcp_server::_conns_mutex);
}

tcp_conn **conns; // 全部已经连接好的连接信息 可以认为是一个数组

// 消息分发路由
msg_router tcp_server::router;

// 创建连接之后要触发的 回调函数
conn_callback tcp_server::conn_start_cb = nullptr;
void *tcp_server::conn_start_cb_args = nullptr;

// 销毁连接之后要触发的 回调函数
conn_callback tcp_server::conn_close_cb = nullptr;
void *tcp_server::conn_close_cb_args = nullptr;

// tcp_server构造函数中 通过event_loop EPOLLIN(有新的socket连接向_socketfd写)
// 触发的 调用accept的函数
void accept_callback(event_loop *loop, int fd, void *args);

// tcp_server的构造函数
// 外界传入的event_loop -> 监控_socketfd -> 有新的连接向_docketfd写 触发EPOLLIN
// -> 调用tcp_server::do_accept() ->
// 从拥有自己的event_loop的thread_pool中取出一个thread去处理新链接 ->
// 每个thread的event_loop会监控 对端的每次读写(EPOLLIN/EPOLLOUT)
tcp_server::tcp_server(event_loop *loop, const char *ip, uint16_t port) {
  memset(&this->_connaddr, 0, sizeof(this->_connaddr));
  // 忽略一些信号 SIGHUP SIGPIPE
  // SIGPIPE:如果客户端关闭，服务端再次write就会产生
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    fprintf(stderr, "signal ignore SIGHUP\n");
  }

  // SIGHUP:如果terminal关闭，会给当前进程发送该信号
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    fprintf(stderr, "signal ignore SIGPIPE\n");
  }

  // 初始化tcp_server的内置_socketfd
  this->_socketfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  if (this->_socketfd == -1) {
    fprintf(stderr, "tcp_server::docket()\n");
    exit(1);
  }

  // 初始化地址
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_aton(ip, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // 可以多次监听 需要设置REUSE
  // 如果在创建socket后没有使用SO_REUSEADDR选项进行设置，再次进行bind就会失败
  int op = 1;
  if (setsockopt(this->_socketfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) <
      0) {
    fprintf(stderr, "setsocketopt SO_REUADDR\n");
  }
  // 绑定端口
  if (bind(_socketfd, (const struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    fprintf(stderr, "bind error\n");
    exit(1);
  }

  // 监听ip端口
  if (listen(this->_socketfd, 500) == -1) {
    fprintf(stderr, "listen error\n");
    exit(1);
  }

  // 初始化tcp_server的event_loop
  _loop = loop;

  // ============== 创建链接管理 ================
  _max_conns = config_file::instance().GetNumber("reactor", "maxConn", 128);
  // 创建链接信息数组
  // 3是因为stdin,stdout,stderr
  // 已经被占用，再新开fd一定是从3开始,所以不加3就会栈溢出
  conns = new tcp_conn *[_max_conns + 3]{nullptr};
  if (conns == nullptr) {
    fprintf(stderr, "new cons[%d] error\n", _max_conns);
    exit(1);
  }

  // TODO qps 测试的时候居然因为这个段错误了: (
  //memset(conns, 0, sizeof(tcp_conn) * (_max_conns + 3));
  //for (int i = 0;i < sizeof(tcp_conn) * (_max_conns + 3); i++) {
  //  printf("%d", *((char*)conns+i));
  //}
  // ==============  创建线程池 ====================
  int thread_cnt = config_file::instance().GetNumber("reactor", "threadNum", 8);
  if (thread_cnt > 0) {
    _thread_pool = new thread_pool(thread_cnt);
    if (_thread_pool == nullptr) {
      fprintf(stderr, "tcp_server new thread_pool error\n");
      exit(1);
    }
  }

  // 将_socketfd 添加到event_loop中
  // 注册_socketfd读事件 --> accept处理
  _loop->add_io_event(this->_socketfd, accept_callback, EPOLLIN, this);
}

void accept_callback(event_loop *loop, int fd, void *args) {
  tcp_server *server = (tcp_server *)args;
  server->do_accept();
}
// tcp_server的析构函数
tcp_server::~tcp_server() { close(this->_socketfd); }

// 循环创建连接
void tcp_server::do_accept() {
  int connfd;
  while (true) {
    // accept与客户端创建链接
#ifdef debug
    printf("begin accept\n");
#endif
    connfd = accept(this->_socketfd, (struct sockaddr *)&this->_connaddr,
                    &this->_addrlen);
    if (connfd == -1) {
      if (errno == EINTR) {
        fprintf(stderr, "accept errno=EINTR\n");
        continue;
      } else if (errno == EMFILE) {
        //建立链接过多，资源不够
        fprintf(stderr, "accept errno=EMFILE\n");
      } else if (errno == EAGAIN) {
        fprintf(stderr, "accept errno=EAGAIN\n");
        break;
      } else {
        fprintf(stderr, "accept error");
        exit(1);
      }
    } else {
      // accecpt succ
      int cur_conns = 0;
      get_conn_num(&cur_conns);

      // 判断链接数量
      if (cur_conns >= _max_conns) {
        fprintf(stderr, "so many connections, max = %d\n", _max_conns);
        close(connfd);
      } else {
        // 判断是否要开启多线程模式
        if (_thread_pool != nullptr) {
          // 选择一个线程来处理
          thread_queue<task_msg> *queue = _thread_pool->get_thread();
          // 创建一个新建链接的消息任务
          task_msg task;
          task.type = task_msg::NEW_CONN;
          task.connfd = connfd;
          // 添加到了消息队列中，让对应的thread进程的event_loop处理
          queue->send(task);
        } else {
          tcp_conn *conn = new tcp_conn(connfd, _loop);
          if (conn == nullptr) {
            fprintf(stderr, "new tcp_conn error\n");
            exit(1);
          }
          // 这样，每次accept成功之后，创建一个与当前客户端套接字绑定的tcp_conn对象。
          // 在构造里就完成了基本的对于EPOLLIN事件的监听和回调动作.
#ifdef debug
          printf("get new connection succ!\n");
#endif
          break;
        }
      }
    }
  }
}
