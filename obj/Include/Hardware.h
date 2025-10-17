#ifndef HARDWARE_H
#define HARDWARE_H

// --- LED 控制 ---
#define LED_COUNT 4 // 开发板上有4个LED

/**
 * @brief 初始化所有硬件设备。
 * @return 0 表示成功, -1 表示失败。
 */
int hardware_init(void);

/**
 * @brief 关闭并释放所有硬件设备。
 */
void hardware_deinit(void);

/**
 * @brief 控制单个 LED 的状态。
 * @param led_num LED 编号 (1 到 4)。
 * @param on      1 表示点亮, 0 表示熄灭。
 * @return 0 表示成功, -1 表示失败。
 */
int control_led(int led_num, int on);

/**
 * @brief 控制蜂鸣器的状态。
 * @param on 1 表示开启, 0 表示关闭。
 * @return 0 表示成功, -1 表示失败。
 */
int control_buzzer(int on);

#endif // HARDWARE_H