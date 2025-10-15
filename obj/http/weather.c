#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "libs/cJSON/cJSON.h"
#include "obj/http/weather.h"

static int resolve_host_ipv4(const char *hostname, char out_ip[INET_ADDRSTRLEN]) {
  struct hostent *host_info = gethostbyname(hostname);
  if (!host_info || !host_info->h_addr_list[0]) return -1;
  const char *ip = inet_ntoa(*(struct in_addr *)host_info->h_addr_list[0]);
  if (!ip) return -1;
  strncpy(out_ip, ip, INET_ADDRSTRLEN - 1);
  out_ip[INET_ADDRSTRLEN - 1] = '\0';
  return 0;
}

int sh_weather_fetch_now(const char *location, const char *key, sh_weather_now_t *out, char *err, size_t errlen) {
  if (!location || !key || !out) {
    if (err && errlen) snprintf(err, errlen, "invalid args");
    return -1;
  }

  char ip[INET_ADDRSTRLEN] = {0};
  if (resolve_host_ipv4("api.seniverse.com", ip) != 0) {
    if (err && errlen) snprintf(err, errlen, "dns fail");
    return -1;
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { if (err && errlen) snprintf(err, errlen, "socket fail"); return -1; }

  struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; addr.sin_port = htons(80); addr.sin_addr.s_addr = inet_addr(ip);
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    if (err && errlen) snprintf(err, errlen, "connect fail");
    close(sockfd); return -1;
  }

  char req[512];
  snprintf(req, sizeof(req),
           "GET /v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c HTTP/1.1\r\n"
           "Host: api.seniverse.com\r\n"
           "Connection: close\r\n\r\n",
           key, location);

  if (write(sockfd, req, strlen(req)) < 0) {
    if (err && errlen) snprintf(err, errlen, "write fail");
    close(sockfd); return -1;
  }

  char buf[8192]; size_t total = 0; ssize_t n;
  while ((n = read(sockfd, buf + total, sizeof(buf) - 1 - total)) > 0) {
    total += (size_t)n; if (total >= sizeof(buf) - 1) break;
  }
  buf[total] = '\0';
  close(sockfd);

  char *body = strstr(buf, "\r\n\r\n");
  if (!body) { if (err && errlen) snprintf(err, errlen, "bad http resp"); return -1; }
  body += 4;

  cJSON *root = cJSON_Parse(body);
  if (!root) { if (err && errlen) snprintf(err, errlen, "json parse fail"); return -1; }

  int rc = -1;
  cJSON *results = cJSON_GetObjectItem(root, "results");
  if (results && cJSON_IsArray(results) && cJSON_GetArraySize(results) > 0) {
    cJSON *item = cJSON_GetArrayItem(results, 0);
    cJSON *loc = cJSON_GetObjectItem(item, "location");
    cJSON *now = cJSON_GetObjectItem(item, "now");
    if (loc && now) {
      cJSON *name = cJSON_GetObjectItem(loc, "name");
      cJSON *text = cJSON_GetObjectItem(now, "text");
      cJSON *code = cJSON_GetObjectItem(now, "code");
      cJSON *tmp = cJSON_GetObjectItem(now, "temperature");
      out->location[0] = out->text[0] = out->code[0] = out->temperature[0] = '\0';
      if (name && cJSON_IsString(name) && name->valuestring)
        strncpy(out->location, name->valuestring, sizeof(out->location)-1);
      if (text && cJSON_IsString(text) && text->valuestring)
        strncpy(out->text, text->valuestring, sizeof(out->text)-1);
      if (code && cJSON_IsString(code) && code->valuestring)
        strncpy(out->code, code->valuestring, sizeof(out->code)-1);
      if (tmp && cJSON_IsString(tmp) && tmp->valuestring)
        strncpy(out->temperature, tmp->valuestring, sizeof(out->temperature)-1);
      rc = 0;
    }
  }

  cJSON_Delete(root);
  if (rc != 0 && err && errlen) snprintf(err, errlen, "json missing fields");
  return rc;
}
