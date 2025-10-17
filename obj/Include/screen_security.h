#ifndef SCREEN_SECURITY_H
#define SCREEN_SECURITY_H

#include "screens_common.h"

void screen_security_build(void);

// 外部事件（如MQTT）或界面操作设置蜂鸣器报警状态，并同步UI高亮
void security_set_alarm_active(int on);

#endif /* SCREEN_SECURITY_H */
