#ifndef SCREENS_COMMON_H
#define SCREENS_COMMON_H

#include "head.h"

void sh_init_styles_once(void);
lv_font_t * sh_get_font_zh(void);

lv_style_t * sh_style_title(void);
lv_style_t * sh_style_card(void);
lv_style_t * sh_style_card_info(void);
lv_style_t * sh_style_nav_text(void);
lv_style_t * sh_style_text_zh(void);
lv_style_t * sh_style_btn_neutral(void);
lv_style_t * sh_style_btn_neutral_pressed(void);

lv_obj_t * sh_create_topbar(const char * title);

const lv_img_dsc_t * sh_weather_icon(const char * code);

#endif /* SCREENS_COMMON_H */
