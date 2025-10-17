#ifndef __DATA_CHAT_H
#define __DATA_CHAT_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "kernel_list.h"

#define MAX_HISTORY 10

// 数据本身：一条消息包含角色和内容
typedef struct {
  char* role;
  char* content;
} ChatData;

// 链表节点结构
typedef struct ChatNode {
  ChatData data;
  struct list_head list;
} ChatNode;

// 函数声明
void data_chat_init_list();                                 // 初始化链表
bool data_chat_add(const char* role, const char* content);  // 添加消息
void data_chat_apply_sliding_window();  // 应用滑动窗口删除多余消息
void data_chat_free();                  // 释放链表内存
char* data_chat_get_history_string();   // 获取历史消息的JSON字符串
int data_chat_get_count();              // 获取当前消息数量

// 将头节点作为全局变量，方便调用
extern ChatNode* chat_head;

#endif