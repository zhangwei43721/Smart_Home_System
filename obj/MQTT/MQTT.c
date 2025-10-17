#include "../Include/mqtt.h"

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../Include/config.h"
#include "libs/cJSON/cJSON.h"
#include "../Include/screen_lighting.h"
#include "../Include/screen_security.h"

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
  if (msg == NULL || msg->payload == NULL) return;

  printf("MQTT: 收到消息 -> 主题: %s | 内容: %s\n", msg->topic,
         (char*)msg->payload);

  // --- 根据不同主题执行不同动作 ---
  if (strcmp(msg->topic, MQTT_SUB_LIGHT_TOPIC) == 0) {
    // --- 灯光控制逻辑 ---
    cJSON* json = cJSON_Parse((const char*)msg->payload);
    if (json == NULL) {
      fprintf(stderr, "MQTT 错误: 解析灯光控制JSON失败。\n");
      return;
    }

    const cJSON* led_obj = cJSON_GetObjectItemCaseSensitive(json, "led");
    const cJSON* state_obj = cJSON_GetObjectItemCaseSensitive(json, "state");

    if (cJSON_IsNumber(led_obj) && cJSON_IsString(state_obj)) {
      int led_num = led_obj->valueint;
      char* state = state_obj->valuestring;
      int on = (strcmp(state, "ON") == 0);

      printf("  -> 动作: 控制 LED%d -> %s\n", led_num, state);
      // 使用照明页面的入口以保持 UI/持久化/硬件 同步
      lighting_set_from_mqtt(led_num, on);
    } else {
      fprintf(stderr, "MQTT 错误: 灯光控制JSON格式不正确。\n");
    }
    cJSON_Delete(json);

  } else if (strcmp(msg->topic, MQTT_SUB_ALARM_TOPIC) == 0) {
    // --- 报警器控制逻辑 ---
    cJSON* json = cJSON_Parse((const char*)msg->payload);
    if (json == NULL) {
      fprintf(stderr, "MQTT 错误: 解析报警器控制JSON失败。\n");
      return;
    }

    const cJSON* state_obj = cJSON_GetObjectItemCaseSensitive(json, "state");

    if (cJSON_IsString(state_obj)) {
      char* state = state_obj->valuestring;
      int on = (strcmp(state, "ON") == 0);

      printf("  -> 动作: 控制蜂鸣器 -> %s\n", state);
      // 调用安防入口以同步 UI 高亮与硬件
      security_set_alarm_active(on);
    } else {
      fprintf(stderr, "MQTT 错误: 报警器控制JSON格式不正确。\n");
    }
    cJSON_Delete(json);
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

  g_mosq = mosquitto_new(NULL, true, NULL);
  if (g_mosq == NULL) {
    fprintf(stderr, "MQTT 错误: Out of memory.\n");
    goto error_cleanup;
  }

  // --- 新增：设置用户名和密码 ---
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
  mosquitto_message_callback_set(g_mosq, on_message);  // <-- 新增：设置消息回调

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