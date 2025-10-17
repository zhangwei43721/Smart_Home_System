// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_chat.h"
#include "openai.h"

// 添加函数声明
char* openai_chat_with_history(ChatNode* history_head, const char* model);

// 定义list_last_entry宏
#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)

int main() {
  const char* api_key = "sk-9b0a6c5e0d8a482cbcdd3c9ea3a1197f";
  const char* model = "deepseek-chat";
  char user_input[2048];

  if (strlen(api_key) == 0) {
    fprintf(stderr, "ERROR: API key is missing.\n");
    return 1;
  }

  // 初始化库和对话历史
  openai_init(api_key);
  data_chat_init_list();

  printf("Welcome! Chat with DeepSeek AI. Type 'exit' to quit.\n");

  while (1) {
    printf("\nYou: ");
    if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
      break;  // 读取失败
    }

    // 移除末尾的换行符
    user_input[strcspn(user_input, "\n")] = 0;

    if (strcmp(user_input, "exit") == 0) {
      printf("Goodbye!\n");
      break;
    }

    // 1. 将用户输入添加到历史
    data_chat_add("user", user_input);

    // 2. 应用滑动窗口，确保历史不超过10条
    data_chat_apply_sliding_window();

    printf("AI is thinking...\n");

    // 3. 使用包含完整历史的函数进行调用
    char* res = openai_chat_with_history(chat_head, model);

    if (res) {
      printf("AI: %s\n", res);

      // 4. 将AI的回复也添加到历史
      data_chat_add("assistant", res);

      // 5. 再次应用滑动窗口
      data_chat_apply_sliding_window();

      free(res);
    } else {
      fprintf(stderr, "ERROR: Failed to get response from AI.\n");
      // 如果请求失败，移除刚刚添加的用户消息，避免错误的上下文
      // (这是一个可选的健壮性设计)
      ChatNode* last_node = list_last_entry(&chat_head->list, ChatNode, list);
      if (strcmp(last_node->data.role, "user") == 0) {
        list_del(&last_node->list);
        free(last_node->data.role);
        free(last_node->data.content);
        free(last_node);
      }
    }
  }

  // 清理资源
  data_chat_free();
  openai_cleanup();
  return 0;
}