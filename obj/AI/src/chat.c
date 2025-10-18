// chat.c
#include "data_chat.h"
#include "openai.h"
#include "openai_internal.h"

extern char* api_key;
extern size_t write_callback(void*, size_t, size_t, void*);

// 函数签名改变：不再接收 prompt，而是接收历史链表的头节点
char* openai_chat_with_history(ChatNode* history_head, const char* model) {
  if (!api_key || !history_head || !model) return NULL;

  CURL* curl = curl_easy_init();
  if (!curl) return NULL;

  struct memory chunk = {malloc(1), 0};
  struct curl_slist* headers = NULL;

  headers = curl_slist_append(headers, "Content-Type: application/json");

  char auth_header[512];
  snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s",
           api_key);
  headers = curl_slist_append(headers, auth_header);

  cJSON* rt = cJSON_CreateObject();
  cJSON_AddStringToObject(rt, "model", model);

  // --- 核心修改：遍历链表，构建 messages 数组 ---
  cJSON* messages = cJSON_AddArrayToObject(rt, "messages");
  ChatNode* p;
  list_for_each_entry(p, &history_head->list, list) {
    cJSON* msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "role", p->data.role);
    cJSON_AddStringToObject(msg, "content", p->data.content);
    cJSON_AddItemToArray(messages, msg);
  }
  // --- 修改结束 ---

  char* json = cJSON_PrintUnformatted(rt);

  curl_easy_setopt(curl, CURLOPT_URL,
                   "https://api.deepseek.com/chat/completions");
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  // 线程环境与弱网健壮性
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);            // 禁止使用信号，避免多线程环境下崩溃
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);     // 连接超时
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);            // 整体超时
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10L);     // 低速超时
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);     // 低速阈值
  char errbuf[CURL_ERROR_SIZE] = {0};
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    if (errbuf[0]) fprintf(stderr, "curl_easy_perform() failed: %s\n", errbuf);
    else fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
  }
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  free(json);
  cJSON_Delete(rt);

  if (res != CURLE_OK) {
    free(chunk.response);
    return NULL;
  }

  cJSON* root = cJSON_Parse(chunk.response);
  if (!root) {
    free(chunk.response);
    return NULL;
  }

  cJSON* choices = cJSON_GetObjectItem(root, "choices");
  cJSON* first_choice = cJSON_GetArrayItem(choices, 0);
  cJSON* message = cJSON_GetObjectItem(first_choice, "message");
  cJSON* content = cJSON_GetObjectItem(message, "content");

  char* result =
      (cJSON_IsString(content)) ? strdup(content->valuestring) : NULL;
  cJSON_Delete(root);
  free(chunk.response);
  return result;
}