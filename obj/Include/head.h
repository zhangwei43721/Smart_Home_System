#ifndef __HEAD_H__
#define __HEAD_H__

#include <stdio.h>
#include <unistd.h>

#define SDL_MAIN_HANDLED
#include "lv_drivers/sdl/sdl.h"
#include "lvgl/lvgl.h"

// 对象、坐标、样式、事件演示
void demo_create_basic_objects(void);
void demo_set_position_and_size(void);
void demo_create_and_apply_style(void);
void demo_add_event_handler(void);
void demo_event_bubbling(void);

// 其他功能演示
void demo_sjpg_from_array(void);
void demo_sjpg_from_file(void);
void demo_freetype_text(void);
void demo_flex_layout(void);


#endif /* __HEAD_H__ */