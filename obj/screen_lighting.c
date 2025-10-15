#include "obj/Include/screen_lighting.h"

void screen_lighting_build(void) {
  sh_init_styles_once();

  lv_obj_t * scr = lv_scr_act();
  lv_obj_t * cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48 - 56);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);

  // 栅格容器：二维小卡（2~3列），可纵向滚动
  lv_obj_t * grid = lv_obj_create(cont);
  lv_obj_remove_style_all(grid);
  lv_obj_set_size(grid, 800 - 24, 480 - 48 - 56 - 24);
  lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
  lv_obj_set_scroll_dir(grid, LV_DIR_VER);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(grid, 12, 0);
  lv_obj_set_style_pad_column(grid, 12, 0);

  const char * rooms[] = {"客厅主灯", "餐厅吊灯", "卧室筒灯", "走廊灯", "书房台灯", "阳台灯", "卫生间灯", "厨房灯"};
  const int n = sizeof(rooms) / sizeof(rooms[0]);

  // 3列布局尺寸（留白与间距考虑）
  int card_w = (800 - 24 - 12 * 2) / 3;  // 3列
  int card_h = 90;

  for (int i = 0; i < n; ++i) {
    lv_obj_t * card = lv_obj_create(grid);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, card_w, card_h);

    // 名称
    lv_obj_t * name = lv_label_create(card);
    lv_label_set_text(name, rooms[i]);
    if (sh_get_font_zh()) lv_obj_set_style_text_font(name, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(name, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
    lv_obj_align(name, LV_ALIGN_TOP_LEFT, 8, 6);

    // 开关
    lv_obj_t * sw = lv_switch_create(card);
    if (i == 0 || i == 2) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -8, 0);
  }

  // 颜色与亮度控制区保持为一张宽卡
  lv_obj_t * ctrl = lv_obj_create(grid);
  lv_obj_add_style(ctrl, sh_style_card(), 0);
  lv_obj_set_size(ctrl, 800 - 24, 160);
  lv_obj_set_flex_flow(ctrl, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ctrl, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

  lv_obj_t * cw = lv_colorwheel_create(ctrl, true);
  lv_obj_set_size(cw, 120, 120);

  lv_obj_t * col = lv_obj_create(ctrl);
  lv_obj_remove_style_all(col);
  lv_obj_set_size(col, 360, 120);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  lv_obj_t * lbl_b = lv_label_create(col);
  lv_label_set_text(lbl_b, "亮度");
  if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl_b, sh_get_font_zh(), 0);
  lv_obj_set_style_text_color(lbl_b, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_t * slider_b = lv_slider_create(col);
  lv_obj_set_width(slider_b, 320);
  lv_slider_set_range(slider_b, 0, 100);
  lv_slider_set_value(slider_b, 80, LV_ANIM_OFF);

  lv_obj_t * lbl_c = lv_label_create(col);
  lv_label_set_text(lbl_c, "色温");
  if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl_c, sh_get_font_zh(), 0);
  lv_obj_set_style_text_color(lbl_c, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_t * slider_c = lv_slider_create(col);
  lv_obj_set_width(slider_c, 320);
  lv_slider_set_range(slider_c, 2700, 6500);
  lv_slider_set_value(slider_c, 4000, LV_ANIM_OFF);
}
