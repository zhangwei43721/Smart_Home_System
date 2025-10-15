#include "obj/Include/screen_scenes.h"

void screen_scenes_build(void) {
  sh_init_styles_once();

  lv_obj_t * scr = lv_scr_act();
  lv_obj_t * cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48 - 56);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);

  // 场景列表（两列卡片）
  lv_obj_t * list = lv_obj_create(cont);
  lv_obj_remove_style_all(list);
  lv_obj_set_size(list, 800 - 24, 480 - 48 - 56 - 24);
  lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
  lv_obj_set_scroll_dir(list, LV_DIR_VER);
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_row(list, 12, 0);
  lv_obj_set_style_pad_column(list, 12, 0);

  const char * names[] = {"到家", "离家", "观影", "晚安", "会客", "阅读"};
  int m = sizeof(names)/sizeof(names[0]);
  int card_w = (800 - 24 - 12) / 2;

  for (int i = 0; i < m; ++i) {
    lv_obj_t * card = lv_obj_create(list);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, card_w, 100);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, names[i]);
    if (sh_get_font_zh()) lv_obj_set_style_text_font(title, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(title, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 6);

    lv_obj_t * btn = lv_btn_create(card);
    lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_size(btn, 120, 36);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "执行");
    lv_obj_center(lbl);
  }
}
