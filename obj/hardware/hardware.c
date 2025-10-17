#include "../Include/Hardware.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

// --- LED 设备定义 ---
#define LED_DEVICE "/dev/Led"
#define LED_MAGIC 'x'
#define LED1_IOCTL _IO(LED_MAGIC, 0)
#define LED2_IOCTL _IO(LED_MAGIC, 1)
#define LED3_IOCTL _IO(LED_MAGIC, 2)
#define LED4_IOCTL _IO(LED_MAGIC, 3)
#define LED_STATE_ON 0
#define LED_STATE_OFF 1

// --- 蜂鸣器设备定义 ---
#define BUZZER_DEVICE "/dev/beep"
#define BUZZER_STATE_ON 0
#define BUZZER_STATE_OFF 1

// 文件描述符
static int g_led_fd = -1;
static int g_buzzer_fd = -1;

/**
 * @brief 初始化所有硬件设备。
 */
int hardware_init(void) {
  // 初始化 LED
  g_led_fd = open(LED_DEVICE, O_RDWR);
  if (g_led_fd < 0) {
    perror("Failed to open LED device");
    return -1;
  }

  // 初始化蜂鸣器
  g_buzzer_fd = open(BUZZER_DEVICE, O_RDWR);
  if (g_buzzer_fd < 0) {
    perror("Failed to open Buzzer device");
    // 如果蜂鸣器打开失败，需要关闭已经打开的LED设备
    if (g_led_fd >= 0) close(g_led_fd);
    return -1;
  }

  // 确保设备在启动时处于关闭状态
  for (int i = 1; i <= LED_COUNT; i++) control_led(i, 0);

  control_buzzer(0);

  return 0;
}

/**
 * @brief 关闭并释放所有硬件设备。
 */
void hardware_deinit(void) {
  if (g_led_fd >= 0) {
    // 关闭前确保所有LED熄灭
    for (int i = 1; i <= LED_COUNT; i++) {
      control_led(i, 0);
    }
    close(g_led_fd);
    g_led_fd = -1;
    printf("Hardware: LED device closed.\n");
  }
  if (g_buzzer_fd >= 0) {
    // 关闭前确保蜂鸣器关闭
    control_buzzer(0);
    close(g_buzzer_fd);
    g_buzzer_fd = -1;
    printf("Hardware: Buzzer device closed.\n");
  }
}

/**
 * @brief 控制单个 LED 的状态。
 */
int control_led(int led_num, int on) {
  if (g_led_fd < 0) return -1;
  if (led_num < 1 || led_num > LED_COUNT) return -1;

  long cmd = 0;
  long state = on ? LED_STATE_ON : LED_STATE_OFF;

  switch (led_num) {
    case 1:
      cmd = LED1_IOCTL;
      break;
    case 2:
      cmd = LED2_IOCTL;
      break;
    case 3:
      cmd = LED3_IOCTL;
      break;
    case 4:
      cmd = LED4_IOCTL;
      break;
    default:
      return -1;
  }

  if (ioctl(g_led_fd, cmd, state) < 0) {
    perror("ioctl failed for LED");
    return -1;
  }
  return 0;
}

/**
 * @brief 控制蜂鸣器的状态。
 */
int control_buzzer(int on) {
  if (g_buzzer_fd < 0) return -1;

  long state = on ? BUZZER_STATE_ON : BUZZER_STATE_OFF;

  if (ioctl(g_buzzer_fd, state, 1) < 0) {
    perror("ioctl failed for Buzzer");
    return -1;
  }
  return 0;
}