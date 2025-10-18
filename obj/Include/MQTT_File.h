#ifndef MQTT_FILE_H
#define MQTT_FILE_H

/**
 * @brief 初始化文件操作模块。
 *        (主要用于确保 media 文件夹存在)
 */
void mqtt_file_init(void);

/**
 * @brief 处理来自 "smart_home/file" 主题的消息。
 *
 * @param payload 收到的消息内容 (应为 JSON 格式)。
 */
void mqtt_file_handle_message(const char* payload);

#endif // MQTT_FILE_H