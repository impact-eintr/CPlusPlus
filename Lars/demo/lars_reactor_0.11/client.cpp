#include "tcp_client.h"
#include <cstdint>
#include <string.h>

void echo(const char *data, uint32_t len, int msgid, net_connection *conn,
          void *user_data) {
  printf("得到服务端回执的数据\n");
  printf("recv server echo: [%s]\n", data);
  printf("msgid: [%d]\n", msgid);
  printf("len: [%d]\n", len);
}

void task_echo(const char *data, uint32_t len, int msgid, net_connection *conn,
          void *user_data) {
  printf("任务\n");
  printf("recv server echo: [%s]\n", data);
  printf("msgid: [%d]\n", msgid);
  printf("len: [%d]\n", len);
}

// 客户端创建的回调
void on_client_build(net_connection *conn, void *args) {
  int msgid = 1;
  const char *msg = "Hello Lars! I'm a client request.";

  conn->send_message(msg, strlen(msg), msgid);
}

// 客户端销毁的回调
void on_client_lost(net_connection *conn, void *args) {
  printf("on_client_lost...\n");
  printf("Client is lost!\n");
}

int main() {
  event_loop loop;

  // 创建tcp客户端
  tcp_client client(&loop, "127.0.0.1", 7777, "clientv0.0.11");

  client.add_msg_router(1, echo);
  client.add_msg_router(101, task_echo);

  // 设置hook函数
  client.set_conn_start(on_client_build);
  client.set_conn_close(on_client_lost);

  loop.event_process();

  return 0;
}
