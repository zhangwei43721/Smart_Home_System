#include "data_chat.h"

#include <stdio.h>

// 定义全局头指针
ChatNode* chat_head = NULL;
static int chat_count = 0;

void data_chat_init_list() {
  chat_head = (ChatNode*)malloc(sizeof(ChatNode));
  if (chat_head != NULL) {
    INIT_LIST_HEAD(&chat_head->list);
    chat_count = 0;
  }
}

// 内部函数，用于创建节点
static ChatNode* data_chat_create_node(const char* role, const char* content) {
  ChatNode* node = (ChatNode*)malloc(sizeof(ChatNode));
  if (node != NULL) {
    // 使用 strdup 复制字符串，确保节点拥有自己的内存
    node->data.role = strdup(role);
    node->data.content = strdup(content);
    if (node->data.role == NULL || node->data.content == NULL) {
      // 如果内存分配失败，清理并返回NULL
      free(node->data.role);
      free(node->data.content);
      free(node);
      return NULL;
    }
    INIT_LIST_HEAD(&node->list);
  }
  return node;
}

bool data_chat_add(const char* role, const char* content) {
  if (chat_head == NULL) return false;

  ChatNode* new_node = data_chat_create_node(role, content);
  if (new_node == NULL) return false;

  list_add_tail(&new_node->list, &chat_head->list);
  chat_count++;
  data_chat_apply_sliding_window();
  return true;
}

// 实现滑动窗口：如果历史超过最大值，则移除最旧的一条
void data_chat_apply_sliding_window() {
  if (chat_head == NULL || list_empty(&chat_head->list)) return;

  while (chat_count > MAX_HISTORY) {
    ChatNode* oldest_node = list_entry(chat_head->list.next, ChatNode, list);
    list_del(&oldest_node->list);

    // 释放节点内字符串的内存
    free(oldest_node->data.role);
    free(oldest_node->data.content);
    // 释放节点本身的内存
    free(oldest_node);

    chat_count--;
  }
}

int data_chat_get_count() { return chat_count; }
char* data_chat_get_history_string() {
    if (chat_head == NULL || list_empty(&chat_head->list)) {
        // 返回一个空的、可被 free 的字符串，避免调用者出错
        char* empty_str = malloc(1);
        empty_str[0] = '\0';
        return empty_str;
    }

    // 1. 计算拼接后的总长度
    size_t total_len = 0;
    ChatNode* p;
    list_for_each_entry(p, &chat_head->list, list) {
        // 格式: "You: message\n" 或 "AI: message\n\n"
        // "You: " = 5, "AI: " = 4, "\n\n" = 2
        total_len += (strcmp(p->data.role, "user") == 0 ? 5 : 4);
        total_len += strlen(p->data.content);
        total_len += 2; // for "\n\n"
    }
    total_len += 1; // for '\0'

    // 2. 分配足够大的内存
    char* history_str = (char*)malloc(total_len);
    if (history_str == NULL) return NULL;
    history_str[0] = '\0'; // 初始化为空字符串

    // 3. 遍历链表并拼接字符串
    list_for_each_entry(p, &chat_head->list, list) {
        if (strcmp(p->data.role, "user") == 0) {
            strcat(history_str, "You: ");
        } else {
            strcat(history_str, "AI: ");
        }
        strcat(history_str, p->data.content);
        strcat(history_str, "\n\n");
    }

    return history_str;
}

void data_chat_free() {
  if (chat_head == NULL) return;

  struct list_head *p, *p_next;
  ChatNode* node;

  list_for_each_safe(p, p_next, &chat_head->list) {
    node = list_entry(p, ChatNode, list);
    list_del(p);
    free(node->data.role);
    free(node->data.content);
    free(node);
  }
  free(chat_head);
  chat_head = NULL;
  chat_count = 0;
}