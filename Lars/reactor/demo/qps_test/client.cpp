#include <cstdint>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <unistd.h>

#include "net_connection.h"
#include "tcp_client.h"
#include "echoMessage.pb.h"

struct Qps{
  Qps() {
    last_time = time(nullptr);
    succ_cnt = 0;
  }
  long last_time; // 最后一次发包时间 ms为单位
  int succ_cnt; // 成功收到服务器回显的次数
};

void echo(const char *data, uint32_t len, int msgid, net_connection *conn, void *user_data) {
  Qps *qps = (Qps*)user_data; // 用户参数

  qps_test::EchoMessage request, response;

  // 解析服务端传来的pb数据
  if (!response.ParseFromArray(data, len)) {
    printf("server call back data error\n");
  }


  std::cout << "获取数据为: " << response.content() << std::endl;
  // 判断数据内容是否与回显一致
  if (response.content() == "Hello Lars!") {
    // 服务器请求响应成功一次
    qps->succ_cnt++;
  }
  long current_time = time(nullptr);
  if (current_time - qps->last_time >= 1) {
    // 如果当前时间比最后记录时间大于1秒，那么我们进行记录
    printf("---> qps = %d <---\n", qps->succ_cnt);
    qps->last_time = current_time; // 记录最后时间
    qps->succ_cnt = 0;             // 清空成功次数
  }

  // 给服务端发送新的请求
  request.set_id(response.id() + 1);
  request.set_content(response.content());

  std::string requestString;
  request.SerializeToString(&requestString);

  conn->send_message(requestString.c_str(), requestString.size(), msgid);
}

// 创建链接成功之后
void connection_start(net_connection *client, void *args) {
  qps_test::EchoMessage request;

  request.set_id(1);
  request.set_content("Hello Lars!");
  std::cout << "发送数据为: " << request.content() << std::endl;

  std::string requestString;

  request.SerializeToString(&requestString);

  int msgid = 1; //与server端的消息路由一致
  client->send_message(requestString.c_str(), requestString.size(), msgid);
}

// 工作线程
void *thread_main(void *args) {
  // 给服务端发送数据包
  event_loop loop;
  tcp_client client(&loop, "127.0.0.1", 7777, "qps client");
  Qps qps;

  // 设置回调
  client.add_msg_router(1, echo, (void*)&qps);

  // 设置链接创建成功之后的Hook
  client.set_conn_start(connection_start);

  loop.event_process();

  return nullptr;
}

// 主函数
int main(int argc, char **argv) {
  if (argc == 1) {
    printf("Usage: ./client [threadNum]\n");
    return 1;
  }
  // 创建N个线程
  int thread_num = atoi(argv[1]);
  pthread_t *tids;
  tids = new pthread_t[thread_num];

  for (int i = 0;i < thread_num; i++) {
    pthread_create(&tids[i], nullptr, thread_main, nullptr);
  }

  for (int i = 0;i < thread_num;i++) {
    pthread_join(tids[i], nullptr);
  }
  return 0;
}
