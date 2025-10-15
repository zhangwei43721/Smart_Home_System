#include "obj/Include/screen_security.h"

void screen_security_build(void) {
  sh_init_styles_once();

  lv_obj_t * scr = lv_scr_act();

  lv_obj_t * cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48 - 56);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_row(cont, 12, 0);
  lv_obj_set_style_pad_column(cont, 12, 0);

  const char * items[] = {"门磁", "红外", "烟雾", "水浸", "窗磁", "震动"};
  for (int i = 0; i < 6; ++i) {
    lv_obj_t * card = lv_obj_create(cont);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, (800 - 24 - 12) / 2, (480 - 48 - 56 - 12*3) / 2);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, items[i]);
    if (sh_get_font_zh()) lv_obj_set_style_text_font(title, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(title, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 6);

    lv_obj_t * status = lv_label_create(card);
    lv_label_set_text(status, (i % 2 == 0) ? "正常" : "告警");
    if (sh_get_font_zh()) lv_obj_set_style_text_font(status, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(status, (i % 2 == 0) ? lv_palette_darken(LV_PALETTE_GREY, 3)
                                                     : lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align(status, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t * btn = lv_btn_create(card);
    lv_obj_set_size(btn, 120, 36);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
    lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "详情");
    if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(lbl, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_center(lbl);
  }
}
