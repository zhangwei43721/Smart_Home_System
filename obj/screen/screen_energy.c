#include "obj/Include/screen_energy.h"

void screen_energy_build(void) {
  sh_init_styles_once();

  lv_obj_t * scr = lv_scr_act();

  lv_obj_t * cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48 - 56);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(cont, 12, 0);

  // 顶部概览卡
  lv_obj_t * card_total = lv_obj_create(cont);
  lv_obj_add_style(card_total, sh_style_card(), 0);
  lv_obj_set_size(card_total, 800 - 24, 90);
  lv_obj_t * t1 = lv_label_create(card_total);
  lv_label_set_text(t1, "今日用电 3.6kWh · 本月 62kWh");
  lv_obj_add_style(t1, sh_style_text_zh_small(), 0);
  lv_obj_set_style_text_color(t1, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_align(t1, LV_ALIGN_LEFT_MID, 8, 0);

  // 简单的分项条形
  const char * names[] = {"空调", "照明", "插座"};
  int values[] = {60, 25, 15};
  for (int i = 0; i < 3; ++i) {
    lv_obj_t * row = lv_obj_create(cont);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 800 - 24, 24);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * name = lv_label_create(row);
    lv_label_set_text_fmt(name, "%s %d%%", names[i], values[i]);
    lv_obj_add_style(name, sh_style_text_zh_small(), 0);
    lv_obj_set_style_text_color(name, lv_palette_darken(LV_PALETTE_GREY, 4), 0);

    lv_obj_t * bar = lv_bar_create(row);
    lv_obj_set_size(bar, 520, 10);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, values[i], LV_ANIM_OFF);
  }
}
