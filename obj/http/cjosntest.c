#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "cJSON.h"

int dns() {
  struct hostent* host_info;
  char* hostname = "api.seniverse.com";

  host_info = gethostbyname(hostname);
  if (host_info == NULL) {
    herror("gethostbyname");
    return -1;
  }

  printf("%s 的 IP 地址列表:\n", hostname);
  for (int i = 0; host_info->h_addr_list[i] != NULL; i++) {
    // inet_ntoa 将网络字节序的地址转换为点分十进制字符串
    printf("  %s\n", inet_ntoa(*(struct in_addr*)host_info->h_addr_list[i]));
  }

  return 0;
}

int main() {
  // 1.套接字
  int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_sockfd == -1) {
    perror("socket fail");
  } else {
    printf("socket success\n");
  }

  // 发起链接
  struct sockaddr_in myaddr;
  bzero(&myaddr, sizeof(myaddr));

  myaddr.sin_family = AF_INET;             // IPV4
  myaddr.sin_port = htons(80);             // 转网络序 http
  myaddr.sin_addr.s_addr = inet_addr("");  // ip，需要用dns解析

  // 2.链接服务器
  int c_ret = connect(tcp_sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
  if (c_ret < 0) {
    perror("connect fail");
  } else {
    printf("connect success\n");
  }

  // 设定请求头
  // https://api.seniverse.com/v3/weather/now.json?key=SirtAO6oBdxoM34w9&location=beijing&language=zh-Hans&unit=c
  char* req =
      "GET "
      "/v3/weather/"
      "now.json?key=SirtAO6oBdxoM34w9&location=guangzhou&language=zh-Hans&unit="
      "c HTTP/1.1\r\nHost:api.seniverse.com\r\n\r\n";
  // 向服务器发送请求 send
  ssize_t w_ret = write(tcp_sockfd, req, strlen(req));
  if (w_ret < 0) {
    perror("write fail");
  } else {
    printf("write req success\n");
  }

  // 读取来自服务器的响应 recv
  char buf[1024] = {0};
  // 读第一次
  ssize_t r_ret = read(tcp_sockfd, buf, sizeof(buf));
  if (r_ret < 0) {
    perror("read fail");
  } else {
    printf("read success\n");
    printf("r_ret--%ld\n", r_ret);
    printf("buf--%s\n", buf);
  }

  bzero(buf, sizeof(buf));  // 清空数组，准备接收下一次数据

  // 读第二次
  r_ret = read(tcp_sockfd, buf, sizeof(buf));
  if (r_ret < 0) {
    perror("read fail");
  } else {
    printf("read success\n");
    printf("r_ret--%ld\n", r_ret);
    printf("buf--%s\n", buf);
  }

  // 把字符串美观转换
  cJSON* object = cJSON_Parse(buf);
  printf("%s\n", cJSON_Print(object));

  // 获取第一层对象results
  cJSON* P1 = cJSON_GetObjectItem(object, "results");  // results是个数组
  cJSON* P2 = cJSON_GetArrayItem(P1, 0);               // 数组第一个对象
  cJSON* P3 = cJSON_GetObjectItem(P2, "now");          // 查找now对象
  cJSON* P4 = cJSON_GetObjectItem(P3, "temperature");  // 找temperature
  printf("%s\n", P4->valuestring);                     // 键值对的打印

  close(tcp_sockfd);
}
