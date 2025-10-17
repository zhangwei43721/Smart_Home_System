#ifndef SCREEN_LIGHTING_H
#define SCREEN_LIGHTING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// 1. 统一且清晰的数据结构
// 描述一个智能灯光设备的所有属性
typedef struct {
    const char* name;       // 灯的名称
    bool is_on;             // 当前是否开启
    bool supports_adjust;   // 是否支持亮度/色温调节
    int brightness;         // 亮度 (0-100)
    int color_temp;         // 色温 (2700-6500)

    // UI关联: 用于在事件回调中快速找到并更新对应的UI控件
    lv_obj_t* card;
} LightDevice;


/**
 * @brief 构建并显示照明控制页面（内部创建容器与布局）
 */
void screen_lighting_build(void);

// 通过外部事件（如MQTT/硬件反馈）设置指定灯的开关状态（1-based: 1..4映射LED1..LED4）
void lighting_set_from_mqtt(int led_index_1_based, int on);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* SCREEN_LIGHTING_H */