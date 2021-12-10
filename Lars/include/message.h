#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "net_connection.h"
#include <ext/hash_map>

// 解决tcp粘包问题的消息头
struct msg_head {
  int msgid;
  int msglen;
};

// 消息头的二进制长度 8
#define MESSAGE_HEAD_LEN 8
// 消息总长度限制
#define MESSAGE_MAX_LEN (65535 - MESSAGE_HEAD_LEN)
// msg 业务回调函数原型
class tcp_clinet;
typedef void msg_callback(const char *data, uint32_t len, int msgid,
                          net_connection *net_conn, void *user_data);

// 消息分发路由机制
class msg_router {
public:
  msg_router() : _router(), _args() {}
  // 给一个消息ID 注册一个对应的回调函数业务
  int register_msg_router(int msgid, msg_callback *msg_cb, void *user_data) {
    // 检查路由是否已经存在
    if (_router.find(msgid) != _router.end()) {
      return -1;
    }
    _router[msgid] = msg_cb;
    _args[msgid] = user_data;
    return 0;
  }

  // 调用某个ID的消息的回调函数
  void call_msg_router(int msgid, uint32_t msglen, const char *data,
                       net_connection *net_conn) {
    // 检查路由是否已经存在
    if (_router.find(msgid) == _router.end()) {
      fprintf(stderr, "msgid %d is not register!\n", msgid);
      return;
    }

    // 直接取出回调函数 执行
    msg_callback *callback = _router[msgid];
    void *user_data = _args[msgid];
    callback(data, msglen, msgid, net_conn, user_data);
  }

private:
  // 针对消息的路由分发 <msgid, callback>
  __gnu_cxx::hash_map<int, msg_callback *> _router;
  // 回调函数对应的参数 <msgid, callback_args>
  __gnu_cxx::hash_map<int, void *> _args;
};

#endif // MESSAGE_H_
