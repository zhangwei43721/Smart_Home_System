#ifndef SH_CONFIG_H
#define SH_CONFIG_H

// 天气配置
// 注意：不要将真实的私钥提交到公共仓库
#define WEATHER_CITY "guangzhou"
#define WEATHER_API_KEY "xxxxxxxxx"

// --- MQTT 配置 ---

// -- 连接设置 --
#define MQTT_BROKER_HOST "127.0.0.1"
#define MQTT_BROKER_PORT 1883
#define MQTT_KEEPALIVE 60
#define MQTT_USERNAME "test"
#define MQTT_PASSWORD "123456"

// -- 发布设置 --
#define MQTT_STARTUP_TOPIC "smart_home/startup"
#define MQTT_STARTUP_MSG "智能家居系统已启动"
#define MQTT_QOS 1
#define MQTT_RETAIN 0

// -- 订阅主题 --
#define MQTT_SUB_LIGHT_TOPIC "smart_home/light"  // 灯光控制
#define MQTT_SUB_ALARM_TOPIC "smart_home/alarm"  // 报警器控制
#define MQTT_SUB_FILE_TOPIC "smart_home/file"    // 文件传输
#define MQTT_SUB_QOS 1                           // 订阅服务质量

// --- OpenAI 配置 ---
// 在本地开发环境中设置 API Key。发布到公共仓库时请留空或通过安全方式注入
#define OPENAI_API_KEY "sk-xxxxxxxxx"

#endif  // SH_CONFIG_H