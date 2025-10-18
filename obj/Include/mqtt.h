#ifndef MQTT_H
#define MQTT_H

/**
 * @brief 初始化并启动 MQTT 客户端，连接到服务器并订阅主题。
 * @return 0 表示成功, 非 0 表示失败。
 */
int mqtt_client_start(void);

/**
 * @brief 断开连接并清理 MQTT 客户端资源。
 */
void mqtt_client_stop(void);

/**
 * @brief 发布一条 MQTT 消息。
 * 
 * @param topic 要发布的主题。
 * @param payload 消息内容。
 * @param qos 服务质量等级 (0, 1, or 2)。
 * @param retain 是否作为保留消息发布 (0 or 1)。
 * @return MOSQ_ERR_SUCCESS 表示成功，其他值表示失败。
 */
int mqtt_publish(const char *topic, const char *payload, int qos, int retain);

/**
 * @brief 发送灯光查询指令（与文件 list 同步的参数格式）。
 * @return 0 表示成功发布请求，非 0 表示失败。
 */
int mqtt_query_light_list(void);

/**
 * @brief 发送报警查询指令（与文件 list 同步的参数格式）。
 * @return 0 表示成功发布请求，非 0 表示失败。
 */
int mqtt_query_alarm_list(void);

#endif // MQTT_H