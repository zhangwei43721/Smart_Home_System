#include "../Include/mqtt.h"

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../Include/MQTT_File.h"
#include "../Include/config.h"
#include "../Include/screen_lighting.h"
#include "../Include/screen_security.h"
#include "../Include/Hardware.h"
#include "../data/state_store.h"
#include "libs/cJSON/cJSON.h"

// --- 静态全局变量 ---
static struct mosquitto* g_mosq = NULL;
static int g_started = 0;

// --- 函数声明 ---
static void on_connect(struct mosquitto* mosq, void* obj, int reason_code);
static void on_publish(struct mosquitto* mosq, void* obj, int mid);
static void on_message(struct mosquitto* mosq, void* obj,
                       const struct mosquitto_message* message);

// --- 回调函数定义 ---

/**
 * @brief 当连接成功建立后被调用。
 */
static void on_connect(struct mosquitto* mosq, void* obj, int reason_code) {
  if (reason_code == 0) {
    printf("MQTT: 成功连接到服务器！\n");

    // 1. 订阅灯光控制主题
    if (mosquitto_subscribe(mosq, NULL, MQTT_SUB_LIGHT_TOPIC, MQTT_SUB_QOS) !=
        MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "MQTT 错误: 订阅灯光主题失败。\n");
    } else {
      printf("MQTT: 已订阅主题 [%s]\n", MQTT_SUB_LIGHT_TOPIC);
    }

    // 2. 订阅报警器控制主题
    if (mosquitto_subscribe(mosq, NULL, MQTT_SUB_ALARM_TOPIC, MQTT_SUB_QOS) !=
        MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "MQTT 错误: 订阅报警器主题失败。\n");
    } else {
      printf("MQTT: 已订阅主题 [%s]\n", MQTT_SUB_ALARM_TOPIC);
    }

    if (mosquitto_subscribe(mosq, NULL, MQTT_SUB_FILE_TOPIC, MQTT_SUB_QOS) !=
        MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "MQTT 错误: 订阅文件主题失败。\n");
    } else {
      printf("MQTT: 已订阅主题 [%s]\n", MQTT_SUB_FILE_TOPIC);
    }

    // 3. 发布一条启动成功消息
    mqtt_publish(MQTT_STARTUP_TOPIC, MQTT_STARTUP_MSG, MQTT_QOS, MQTT_RETAIN);

  } else {
    // mosquitto_connack_string() 对 v3.x 客户端更友好
    fprintf(stderr, "MQTT: 连接失败: %s\n",
            mosquitto_connack_string(reason_code));
  }
}

/**
 * @brief 当有新消息到达已订阅的主题时被调用。
 */
static void on_message(struct mosquitto* mosq, void* obj,
                       const struct mosquitto_message* msg) {
  // 1. 基本检查
  if (msg == NULL || msg->payload == NULL) return;

  // 2. 打印标准日志，方便追踪
  printf("MQTT: 收到消息 -> 主题: %s | 内容: %s\n", msg->topic,
         (char*)msg->payload);

  // 3. 根据主题分发任务
  if (strcmp(msg->topic, MQTT_SUB_LIGHT_TOPIC) == 0) {
    // --- 灯光控制逻辑 ---
    cJSON* json = cJSON_Parse((const char*)msg->payload);
    if (json) {
      const cJSON* cmd = cJSON_GetObjectItemCaseSensitive(json, "command");
      if (cJSON_IsString(cmd) && cmd->valuestring && strcmp(cmd->valuestring, "list") == 0) {
        LightPersist lp[LED_COUNT];
        memset(lp, 0, sizeof(lp));
        ss_lighting_load(lp, LED_COUNT);
        cJSON *root = cJSON_CreateObject();
        if (root) {
          cJSON_AddStringToObject(root, "command", "list_response");
          cJSON_AddStringToObject(root, "status", "ok");
          cJSON *arr = cJSON_CreateArray();
          if (arr) {
            for (int i = 0; i < LED_COUNT; ++i) {
              cJSON *item = cJSON_CreateObject();
              if (item) {
                cJSON_AddNumberToObject(item, "id", i + 1);
                cJSON_AddStringToObject(item, "state", lp[i].is_on ? "ON" : "OFF");
                cJSON_AddNumberToObject(item, "brightness", lp[i].brightness);
                cJSON_AddNumberToObject(item, "color_temp", lp[i].color_temp);
                cJSON_AddItemToArray(arr, item);
              }
            }
            cJSON_AddItemToObject(root, "lights", arr);
          }
          char *payload = cJSON_PrintUnformatted(root);
          if (payload) { mqtt_publish(MQTT_SUB_LIGHT_TOPIC, payload, MQTT_QOS, 0); free(payload); }
          cJSON_Delete(root);
        }
        cJSON_Delete(json);
        return;
      }
      const cJSON* led_obj = cJSON_GetObjectItemCaseSensitive(json, "led");
      const cJSON* state_obj = cJSON_GetObjectItemCaseSensitive(json, "state");
      if (cJSON_IsNumber(led_obj) && cJSON_IsString(state_obj)) {
        lighting_set_from_mqtt(led_obj->valueint,
                               strcmp(state_obj->valuestring, "ON") == 0);
      }
      cJSON_Delete(json);
    }
  } else if (strcmp(msg->topic, MQTT_SUB_ALARM_TOPIC) == 0) {
    // --- 报警器控制逻辑 ---
    cJSON* json = cJSON_Parse((const char*)msg->payload);
    if (json) {
      const cJSON* cmd = cJSON_GetObjectItemCaseSensitive(json, "command");
      if (cJSON_IsString(cmd) && cmd->valuestring && strcmp(cmd->valuestring, "list") == 0) {
        int alarm_on = ss_alarm_load();
        int sec_state = ss_security_load();
        cJSON *root = cJSON_CreateObject();
        if (root) {
          cJSON_AddStringToObject(root, "command", "list_response");
          cJSON_AddStringToObject(root, "status", "ok");
          cJSON_AddStringToObject(root, "state", alarm_on ? "ON" : "OFF");
          cJSON_AddNumberToObject(root, "mode", sec_state);
          char *payload = cJSON_PrintUnformatted(root);
          if (payload) { mqtt_publish(MQTT_SUB_ALARM_TOPIC, payload, MQTT_QOS, 0); free(payload); }
          cJSON_Delete(root);
        }
        cJSON_Delete(json);
        return;
      }
      const cJSON* state_obj = cJSON_GetObjectItemCaseSensitive(json, "state");
      if (cJSON_IsString(state_obj)) {
        security_set_alarm_active(strcmp(state_obj->valuestring, "ON") == 0);
      }
      cJSON_Delete(json);
    }
  } else if (strcmp(msg->topic, MQTT_SUB_FILE_TOPIC) == 0) {
    // --- 文件操作逻辑 ---
    mqtt_file_handle_message((const char*)msg->payload);
  }
}

/**
 * @brief 当消息成功发布后被调用。
 */
static void on_publish(struct mosquitto* mosq, void* obj, int mid) {
  printf("MQTT: 消息 (mid: %d) 已成功发布。\n", mid);
}

// --- 公共接口函数 ---

/**
 * @brief 初始化并启动 MQTT 客户端。
 */
int mqtt_client_start(void) {
  int rc;

  if (g_started) return 0;

  mosquitto_lib_init();
  mqtt_file_init();

  g_mosq = mosquitto_new(NULL, true, NULL);
  if (g_mosq == NULL) {
    fprintf(stderr, "MQTT 错误: Out of memory.\n");
    goto error_cleanup;
  }

  // 设置用户名和密码 ---
  if (MQTT_USERNAME && MQTT_USERNAME[0] != '\0') {
    rc = mosquitto_username_pw_set(g_mosq, MQTT_USERNAME, MQTT_PASSWORD);
    if (rc != MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "MQTT 错误: 设置用户名和密码失败: %s\n",
              mosquitto_strerror(rc));
      goto error_cleanup;
    }
  }

  // --- 设置所有回调函数 ---
  mosquitto_connect_callback_set(g_mosq, on_connect);
  mosquitto_publish_callback_set(g_mosq, on_publish);
  mosquitto_message_callback_set(g_mosq, on_message);  // 设置消息回调

  // 连接服务器
  rc = mosquitto_connect(g_mosq, MQTT_BROKER_HOST, MQTT_BROKER_PORT,
                         MQTT_KEEPALIVE);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "MQTT 错误: 无法连接到服务器: %s\n",
            mosquitto_strerror(rc));
    goto error_cleanup;
  }

  // 在后台线程中运行网络循环
  rc = mosquitto_loop_start(g_mosq);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "MQTT 错误: 启动循环线程失败: %s\n",
            mosquitto_strerror(rc));
    mosquitto_disconnect(g_mosq);  // loop_start 失败后，需要手动断连
    goto error_cleanup;
  }

  g_started = 1;
  printf("MQTT 客户端已启动。\n");
  return 0;

error_cleanup:
  if (g_mosq) {
    mosquitto_destroy(g_mosq);
    g_mosq = NULL;
  }
  mosquitto_lib_cleanup();
  return 1;
}

/**
 * @brief 停止并清理 MQTT 客户端。
 */
void mqtt_client_stop(void) {
  if (!g_started) return;

  if (g_mosq) {
    mosquitto_disconnect(g_mosq);
    mosquitto_loop_stop(g_mosq, true);  // 等待后台线程结束
    mosquitto_destroy(g_mosq);
    g_mosq = NULL;
  }
  mosquitto_lib_cleanup();
  g_started = 0;
  printf("MQTT 客户端已停止。\n");
}

/**
 * @brief 发布一条 MQTT 消息。
 */
int mqtt_publish(const char* topic, const char* payload, int qos, int retain) {
  if (!g_started || g_mosq == NULL) return MOSQ_ERR_NO_CONN;

  int len = (payload) ? (int)strlen(payload) : 0;
  printf("MQTT: 准备发布消息 -> 主题: %s\n", topic);
  return mosquitto_publish(g_mosq, NULL, topic, len, payload, qos,
                           retain ? true : false);
}