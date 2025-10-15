#include "obj/Include/screens_common.h"
#include "obj/Include/screen_ac.h"

// 外部导航（在 screens_home.c）
extern void demo_dashboard(void);

static void on_back(lv_event_t *e) {
  LV_UNUSED(e);
  demo_dashboard();
}

void screen_ac_show(void) {
  // 清屏并创建顶栏
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  lv_obj_t *top = sh_create_topbar("空调 Air Conditioner");
  LV_UNUSED(top);

  // 内容容器
  lv_obj_t * cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);

  // 简单卡片：模式/风速占位
  lv_obj_t *card = lv_obj_create(cont);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 300);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *title = lv_label_create(card);
  lv_obj_add_style(title, sh_style_text_zh(), 0);
  lv_obj_set_style_text_color(title, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_label_set_text(title, "模式/风速/摆风（占位，后续完善）");
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 8);

  // 左上角圆形返回按钮
  lv_obj_t * btn_back = lv_btn_create(scr);
  lv_obj_set_size(btn_back, 44, 44);
  lv_obj_set_style_radius(btn_back, 22, 0);
  lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 6, 6);
  lv_obj_add_event_cb(btn_back, on_back, LV_EVENT_CLICKED, NULL);
  lv_obj_t * lbl = lv_label_create(btn_back);
  lv_label_set_text(lbl, "←");
  lv_obj_center(lbl);
}
