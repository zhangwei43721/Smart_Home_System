#include "../Include/MQTT_File.h"

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../Include/config.h"
#include "../Include/mqtt.h"
#include "libs/cJSON/cJSON.h"

#define MEDIA_DIR "./media"
#define FILE_PATH_MAX_LEN 256
#define CHUNK_SIZE 3072

// --- Base64 辅助函数 ---
static const char b64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static size_t base64_encode(const unsigned char* data, size_t input_length,
                            char* encoded_data) {
  size_t output_length = 4 * ((input_length + 2) / 3);
  if (encoded_data == NULL) return output_length;

  for (size_t i = 0, j = 0; i < input_length;) {
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;
    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
    encoded_data[j++] = b64_chars[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = b64_chars[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = b64_chars[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = b64_chars[(triple >> 0 * 6) & 0x3F];
  }
  static const int mod_table[] = {0, 2, 1};
  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[output_length - 1 - i] = '=';
  encoded_data[output_length] = '\0';
  return output_length;
}

/**
 * @brief Base64 解码函数。
 */
static size_t base64_decode(const char* data, unsigned char* decoded_data) {
  size_t input_length = strlen(data);
  if (input_length % 4 != 0) return 0;
  size_t output_length = input_length / 4 * 3;
  if (data[input_length - 1] == '=') (output_length)--;
  if (data[input_length - 2] == '=') (output_length)--;
  if (decoded_data == NULL) return output_length;
  char* b64_inv = (char*)malloc(256);
  if (b64_inv == NULL) return 0;
  memset(b64_inv, 0, 256);
  for (size_t i = 0; i < 64; i++) b64_inv[(unsigned char)b64_chars[i]] = i;
  for (size_t i = 0, j = 0; i < input_length;) {
    uint32_t sextet_a =
        data[i] == '=' ? 0 & i++ : b64_inv[(unsigned char)data[i++]];
    uint32_t sextet_b =
        data[i] == '=' ? 0 & i++ : b64_inv[(unsigned char)data[i++]];
    uint32_t sextet_c =
        data[i] == '=' ? 0 & i++ : b64_inv[(unsigned char)data[i++]];
    uint32_t sextet_d =
        data[i] == '=' ? 0 & i++ : b64_inv[(unsigned char)data[i++]];
    uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) +
                      (sextet_c << 1 * 6) + (sextet_d << 0 * 6);
    if (j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }
  free(b64_inv);
  return output_length;
}

/**
 * @brief 发送响应消息的辅助函数。
 */
static void send_response(const char* command, const char* filename,
                          const char* status, const char* message) {
  cJSON* root = cJSON_CreateObject();
  if (!root) return;
  cJSON_AddStringToObject(root, "command", command);
  if (filename) cJSON_AddStringToObject(root, "filename", filename);
  cJSON_AddStringToObject(root, "status", status);
  if (message) cJSON_AddStringToObject(root, "message", message);
  char* response_payload = cJSON_PrintUnformatted(root);
  if (response_payload) {
    mqtt_publish(MQTT_SUB_FILE_TOPIC, response_payload, MQTT_QOS, 0);
    free(response_payload);
  }
  cJSON_Delete(root);
}

/**
 * @brief 处理“获取文件列表”请求。
 */
static void handle_list_files() {
  printf("DEBUG: handle_list_files() 被调用.\n");

  cJSON* root = cJSON_CreateObject();
  if (!root) {
    fprintf(stderr, "ERROR: 创建 cJSON 根对象失败。\n");
    return;
  }

  cJSON_AddStringToObject(root, "command", "list_response");

  cJSON* files_array = cJSON_CreateArray();
  if (!files_array) {
    fprintf(stderr, "ERROR: 创建 cJSON 文件数组失败。\n");
    cJSON_Delete(root);
    return;
  }
  cJSON_AddItemToObject(root, "files", files_array);

  DIR* d = opendir(MEDIA_DIR);
  if (d) {
    printf("DEBUG: 目录 '%s' 打开成功。\n", MEDIA_DIR);
    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type == DT_REG) {  // 只列出普通文件
        cJSON_AddItemToArray(files_array, cJSON_CreateString(dir->d_name));
        printf("DEBUG: 找到文件: %s\n", dir->d_name);
      }
    }
    closedir(d);
    cJSON_AddStringToObject(root, "status", "ok");
  } else {
    perror("ERROR: opendir() failed");  // 使用 perror 打印系统错误信息
    cJSON_AddStringToObject(root, "status", "error");
    cJSON_AddStringToObject(root, "message", "Cannot open media directory.");
  }

  char* response_payload = cJSON_PrintUnformatted(root);

  if (response_payload) {
    printf("DEBUG: Publishing list_response: %s\n", response_payload);
    mqtt_publish(MQTT_SUB_FILE_TOPIC, response_payload, MQTT_QOS, 0);
    free(response_payload);  // mosquitto_publish
                             // 会复制内容，所以在这里释放是安全的
  } else {
    fprintf(stderr,
            "ERROR: Failed to generate JSON string for list_response.\n");
  }

  cJSON_Delete(root);
}

/**
 * @brief 处理“下载文件”请求。
 */
static void handle_get_file(const char* filename) {
  if (!filename) return;
  char filepath[FILE_PATH_MAX_LEN];
  snprintf(filepath, sizeof(filepath), "%s/%s", MEDIA_DIR, filename);
  FILE* file = fopen(filepath, "rb");
  if (!file) {
    send_response("get_response", filename, "error", "File not found.");
    return;
  }

  unsigned char buffer[CHUNK_SIZE];
  size_t bytes_read;
  int seq = 0;
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  do {
    bytes_read = fread(buffer, 1, CHUNK_SIZE, file);
    if (bytes_read > 0) {
      size_t encoded_len = base64_encode(buffer, bytes_read, NULL);
      char* encoded_data = (char*)malloc(encoded_len + 1);
      if (encoded_data) {
        base64_encode(buffer, bytes_read, encoded_data);
        cJSON* root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "command", "get_response");
        cJSON_AddStringToObject(root, "filename", filename);
        cJSON_AddStringToObject(root, "status", "ok");
        cJSON_AddStringToObject(root, "payload", encoded_data);
        cJSON_AddNumberToObject(root, "seq", seq++);
        cJSON_AddBoolToObject(root, "eof", (ftell(file) >= file_size));
        char* response_payload = cJSON_PrintUnformatted(root);
        if (response_payload) {
          mqtt_publish(MQTT_SUB_FILE_TOPIC, response_payload, MQTT_QOS, 0);
          free(response_payload);
        }
        cJSON_Delete(root);
        free(encoded_data);
      }
    }
  } while (bytes_read > 0);
  fclose(file);
}

/**
 * @brief 处理“上传文件”请求。
 */
static void handle_put_file(cJSON* json_root) {
  const cJSON* filename_obj =
      cJSON_GetObjectItemCaseSensitive(json_root, "filename");
  const cJSON* payload_obj =
      cJSON_GetObjectItemCaseSensitive(json_root, "payload");
  const cJSON* seq_obj = cJSON_GetObjectItemCaseSensitive(json_root, "seq");
  const cJSON* eof_obj = cJSON_GetObjectItemCaseSensitive(json_root, "eof");

  if (!cJSON_IsString(filename_obj) || !cJSON_IsString(payload_obj) ||
      !cJSON_IsNumber(seq_obj) || !cJSON_IsBool(eof_obj)) {
    send_response("put_response", NULL, "error",
                  "Invalid JSON format for put.");
    return;
  }

  char filepath[FILE_PATH_MAX_LEN];
  snprintf(filepath, sizeof(filepath), "%s/%s", MEDIA_DIR,
           filename_obj->valuestring);

  const char* mode = (seq_obj->valueint == 0) ? "wb" : "ab";
  FILE* file = fopen(filepath, mode);
  if (!file) {
    send_response("put_response", filename_obj->valuestring, "error",
                  "Failed to open file for writing.");
    return;
  }

  size_t decoded_len = base64_decode(payload_obj->valuestring, NULL);
  unsigned char* decoded_data = (unsigned char*)malloc(decoded_len);
  if (decoded_data) {
    size_t written_len = base64_decode(payload_obj->valuestring, decoded_data);
    fwrite(decoded_data, 1, written_len, file);
    free(decoded_data);
  }
  fclose(file);

  if (cJSON_IsTrue(eof_obj)) {
    send_response("put_response", filename_obj->valuestring, "ok",
                  "File uploaded successfully.");
  }
}

/**
 * @brief 处理“删除文件”请求。
 */
static void handle_delete_file(const char* filename) {
  if (!filename) return;
  char filepath[FILE_PATH_MAX_LEN];
  snprintf(filepath, sizeof(filepath), "%s/%s", MEDIA_DIR, filename);
  if (remove(filepath) == 0) {
    send_response("delete_response", filename, "ok", NULL);
  } else {
    send_response("delete_response", filename, "error",
                  "File not found or permission denied.");
  }
}

// --- 公共接口函数 ---

/**
 * @brief 初始化文件操作模块。
 */
void mqtt_file_init(void) {
  struct stat st = {0};
  if (stat(MEDIA_DIR, &st) == -1) {
    if (mkdir(MEDIA_DIR, 0755) == 0) {
      printf("INFO: Created directory: %s\n", MEDIA_DIR);
    } else {
      perror("ERROR: Failed to create directory");
    }
  }
}

/**
 * @brief 处理来自 "smart_home/file" 主题的消息。
 */
void mqtt_file_handle_message(const char* payload) {
  cJSON* root = cJSON_Parse(payload);
  if (root == NULL) {
    fprintf(stderr, "MQTT File: Failed to parse JSON.\n");
    return;
  }
  const cJSON* command_obj = cJSON_GetObjectItemCaseSensitive(root, "command");
  if (!cJSON_IsString(command_obj) || (command_obj->valuestring == NULL)) {
    fprintf(stderr, "MQTT File: 'command' field is missing or not a string.\n");
    cJSON_Delete(root);
    return;
  }

  char* command = command_obj->valuestring;
  printf("MQTT File: Received command -> %s\n", command);

  if (strcmp(command, "list") == 0) {
    handle_list_files();
  } else {
    const cJSON* filename_obj =
        cJSON_GetObjectItemCaseSensitive(root, "filename");
    char* filename =
        (cJSON_IsString(filename_obj)) ? filename_obj->valuestring : NULL;
    if (strcmp(command, "get") == 0) {
      handle_get_file(filename);
    } else if (strcmp(command, "put") == 0) {
      handle_put_file(root);
    } else if (strcmp(command, "delete") == 0) {
      handle_delete_file(filename);
    } else {
      fprintf(stderr, "MQTT File: Unknown command '%s'.\n", command);
    }
  }
  cJSON_Delete(root);
}