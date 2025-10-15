#include "obj/Include/screen_climate.h"

void screen_climate_build(void) {
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

  // 温控主卡
  lv_obj_t * card = lv_obj_create(cont);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 150);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t * t = lv_label_create(card);
  lv_label_set_text(t, "客厅温度 24℃ (制冷)");
  if (sh_get_font_zh()) lv_obj_set_style_text_font(t, sh_get_font_zh(), 0);
  lv_obj_set_style_text_color(t, lv_palette_darken(LV_PALETTE_GREY, 4), 0);

  // 温度调节
  static int16_t target = 24;
  lv_obj_t * row = lv_obj_create(card);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, 220, 60);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t * btn_dec = lv_btn_create(row);
  lv_obj_add_style(btn_dec, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_dec, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_size(btn_dec, 56, 40);
  lv_obj_t * lbnd = lv_label_create(btn_dec);
  lv_label_set_text(lbnd, "-");
  lv_obj_center(lbnd);

  lv_obj_t * temp = lv_label_create(row);
  lv_label_set_text_fmt(temp, "%d℃", target);
  if (sh_get_font_zh()) lv_obj_set_style_text_font(temp, sh_get_font_zh(), 0);
  lv_obj_set_style_text_color(temp, lv_palette_darken(LV_PALETTE_GREY, 4), 0);

  lv_obj_t * btn_inc = lv_btn_create(row);
  lv_obj_add_style(btn_inc, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_inc, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_size(btn_inc, 56, 40);
  lv_obj_t * lbni = lv_label_create(btn_inc);
  lv_label_set_text(lbni, "+");
  lv_obj_center(lbni);

  // 模式选择
  lv_obj_t * mode = lv_dropdown_create(card);
  lv_dropdown_set_options_static(mode, "制冷\n制热\n送风\n除湿\n自动");
  lv_obj_set_width(mode, 120);
}
