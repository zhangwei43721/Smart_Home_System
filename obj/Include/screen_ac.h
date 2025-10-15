#ifndef SCREEN_AC_H
#define SCREEN_AC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// --- 1. 使用枚举定义空调的模式和风速，代码更清晰 ---
typedef enum {
    AC_MODE_COOL,
    AC_MODE_HEAT,
    AC_MODE_FAN,
    AC_MODE_DRY,
} ACMode;

typedef enum {
    AC_FAN_AUTO,
    AC_FAN_LOW,
    AC_FAN_MID,
    AC_FAN_HIGH,
} ACFanSpeed;

// --- 2. 完整的空调状态数据结构 ---
typedef struct {
    bool is_on;                 // 空调是否开启
    int temperature_target;     // 目标温度 (16-30)
    int temperature_current;    // 当前室温 (用于显示)
    ACMode mode;                // 当前模式
    ACFanSpeed fan_speed;       // 当前风速
    bool swing_on;              // 摆风是否开启

    // --- UI关联 ---
    // 将相关的UI控件指针保存在这里，方便刷新
    struct {
        lv_obj_t* label_temp_target;
        lv_obj_t* btn_power;
        lv_obj_t* mode_btns[4];
        lv_obj_t* fan_btns[4];
        lv_obj_t* swing_switch;
        // 存放所有可禁用的控件
        lv_obj_t* controls_panel; 
    } ui;

} ACState;


/**
 * @brief 构建并显示功能完整的空调控制页面
 * 
 * @note  此函数会清空当前屏幕内容
 */
void screen_ac_build(void);


#ifdef __cplusplus
}
#endif

#endif // SCREEN_AC_H