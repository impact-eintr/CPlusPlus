#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

class net_connection {
public:
  // 发送消息的接口
  virtual int send_message(const char *data, int datalen, int cmdid) = 0;
  // 获取通信文件描述符的接口
  virtual int get_fd() = 0;
};

// 创建/销毁连接时触发的回调函数类型
typedef void (*conn_callback)(net_connection *conn, void *args);

#endif // NET_CONNECTION_H_
