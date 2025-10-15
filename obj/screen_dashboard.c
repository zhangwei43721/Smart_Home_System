#include "obj/Include/screen_dashboard.h"
// JoyPixels emoji C 数组（仅使用无连字符的符号名，避免编译问题）
extern const lv_img_dsc_t cloud;
extern const lv_img_dsc_t bolt;
extern const lv_img_dsc_t thermometer;
extern const lv_img_dsc_t home;
extern const lv_img_dsc_t door;
extern const lv_img_dsc_t film;

void screen_dashboard_build(void) {
  sh_init_styles_once();

  lv_obj_t * scr = lv_scr_act();
  // 背景容器（浅灰）、占满内容区
  lv_obj_t * cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48 - 56);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);

  // 可滚动列表容器（纵向滚动 + 两列卡片自动换行）
  lv_obj_t * list = lv_obj_create(cont);
  lv_obj_remove_style_all(list);
  lv_obj_set_size(list, 800 - 24, 480 - 48 - 56 - 24);
  lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(list, 12, 0);
  lv_obj_set_style_pad_column(list, 12, 0);
  // list 默认即可滚动，确保垂直滚动开启
  lv_obj_set_scroll_dir(list, LV_DIR_VER);

  // 顶部：天气信息（淡蓝信息卡）
  lv_obj_t * card_weather = lv_obj_create(list);
  lv_obj_add_style(card_weather, sh_style_card_info(), 0);
  lv_obj_set_size(card_weather, 800 - 24, 80);
  lv_obj_t * w_icon = lv_img_create(card_weather);
  lv_img_set_src(w_icon, sh_weather_icon("cloud"));
  lv_img_set_zoom(w_icon, 114);
  lv_obj_align(w_icon, LV_ALIGN_LEFT_MID, 8, 0);

  lv_obj_t * w_city = lv_label_create(card_weather);
  lv_obj_add_style(w_city, sh_style_text_zh(), 0);
  lv_label_set_text(w_city, "上海 · 多云 26℃ · 空气优");
  lv_obj_align_to(w_city, w_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  // 能耗概览（宽卡）
  lv_obj_t * card_energy = lv_obj_create(list);
  lv_obj_add_style(card_energy, sh_style_card(), 0);
  lv_obj_set_size(card_energy, 800 - 24, 90);
  lv_obj_t * e_icon = lv_img_create(card_energy);
  lv_img_set_src(e_icon, &bolt);
  lv_img_set_zoom(e_icon, 114);
  lv_obj_align(e_icon, LV_ALIGN_LEFT_MID, 8, 0);

  lv_obj_t * lbl_e = lv_label_create(card_energy);
  lv_label_set_text(lbl_e, "今日用电 3.6kWh · 本月 62kWh");
  lv_obj_add_style(lbl_e, sh_style_text_zh(), 0);
  lv_obj_set_style_text_color(lbl_e, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_align_to(lbl_e, e_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  // 次行：空调温控卡（半宽）
  lv_obj_t * card_climate = lv_obj_create(list);
  lv_obj_add_style(card_climate, sh_style_card(), 0);
  lv_obj_set_size(card_climate, (800 - 24 - 12) / 2, 140);
  lv_obj_t * c_icon = lv_img_create(card_climate);
  lv_img_set_src(c_icon, &thermometer);
  lv_img_set_zoom(c_icon, 114);
  lv_obj_align(c_icon, LV_ALIGN_TOP_LEFT, 8, 4);

  lv_obj_t * lbl_c = lv_label_create(card_climate);
  lv_label_set_text(lbl_c, "空调 · 客厅 24℃");
  lv_obj_add_style(lbl_c, sh_style_text_zh(), 0);
  lv_obj_set_style_text_color(lbl_c, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_align_to(lbl_c, c_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  lv_obj_t * row_c = lv_obj_create(card_climate);
  lv_obj_remove_style_all(row_c);
  lv_obj_set_size(row_c, ((800 - 24 - 12) / 2) - 16, 40);
  lv_obj_align(row_c, LV_ALIGN_BOTTOM_LEFT, 8, -8);
  lv_obj_set_flex_flow(row_c, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row_c, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t * btn_c_dec = lv_btn_create(row_c);
  lv_obj_add_style(btn_c_dec, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_c_dec, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_size(btn_c_dec, 56, 36);
  lv_obj_t * lbd = lv_label_create(btn_c_dec);
  lv_label_set_text(lbd, "-");
  lv_obj_center(lbd);
  lv_obj_t * temp = lv_label_create(row_c);
  lv_label_set_text(temp, "24℃");
  if (sh_get_font_zh()) lv_obj_set_style_text_font(temp, sh_get_font_zh(), 0);
  lv_obj_set_style_text_color(temp, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_t * btn_c_inc = lv_btn_create(row_c);
  lv_obj_add_style(btn_c_inc, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_c_inc, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_size(btn_c_inc, 56, 36);
  lv_obj_t * lbi = lv_label_create(btn_c_inc);
  lv_label_set_text(lbi, "+");
  lv_obj_center(lbi);

  // 快捷场景（半宽卡，与空调并排）
  lv_obj_t * card_scene = lv_obj_create(list);
  lv_obj_add_style(card_scene, sh_style_card(), 0);
  lv_obj_set_size(card_scene, (800 - 24 - 12) / 2, 140);
  lv_obj_t * s_icon = lv_img_create(card_scene);
  lv_img_set_src(s_icon, &home);
  lv_img_set_zoom(s_icon, 114);
  lv_obj_align(s_icon, LV_ALIGN_TOP_LEFT, 8, 4);

  lv_obj_t * lbl_s = lv_label_create(card_scene);
  lv_label_set_text(lbl_s, "快捷场景");
  lv_obj_add_style(lbl_s, sh_style_text_zh(), 0);
  lv_obj_set_style_text_color(lbl_s, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_align_to(lbl_s, s_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  lv_obj_t * row_s = lv_obj_create(card_scene);
  lv_obj_remove_style_all(row_s);
  lv_obj_set_size(row_s, ((800 - 24 - 12) / 2) - 16, 40);
  lv_obj_align(row_s, LV_ALIGN_BOTTOM_LEFT, 8, -8);
  lv_obj_set_flex_flow(row_s, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row_s, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  const char * qscenes[] = {"到家", "离家", "观影"};
  for (int qi = 0; qi < 3; ++qi) {
    lv_obj_t * b = lv_btn_create(row_s);
    lv_obj_add_style(b, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(b, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_size(b, 72, 36);
    // 图标 + 文本
    lv_obj_t * rowi = lv_obj_create(b);
    lv_obj_remove_style_all(rowi);
    lv_obj_set_size(rowi, 68, 30);
    lv_obj_set_flex_flow(rowi, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(rowi, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * ib = lv_img_create(rowi);
    if (qi == 0) lv_img_set_src(ib, &home);
    else if (qi == 1) lv_img_set_src(ib, &door);
    else lv_img_set_src(ib, &film);
    lv_img_set_zoom(ib, 114);

    lv_obj_t * lb = lv_label_create(rowi);
    lv_obj_add_style(lb, sh_style_text_zh(), 0);
    lv_label_set_text(lb, qscenes[qi]);
  }

  const char * titles[] = {
    "照明·15盏开启","安防·已布防","环境·PM2.5 良","窗帘·50%",
    "门锁·已上锁","设备·在线12台"
  };
  const int count = sizeof(titles)/sizeof(titles[0]);

  for (int i = 0; i < count; ++i) {
    lv_obj_t * card = lv_obj_create(list);
    lv_obj_add_style(card, sh_style_card(), 0);
    // 两列卡片，自动换行（宽度按照两列留白计算）
    lv_obj_set_size(card, (800 - 24 - 12) / 2, 140);

    lv_obj_t * title = lv_label_create(card);
    if (sh_get_font_zh()) lv_obj_set_style_text_font(title, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(title, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
    lv_label_set_text(title, titles[i]);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 6);

    // 右下角按钮：进入
    lv_obj_t * btn = lv_btn_create(card);
    lv_obj_set_size(btn, 120, 36);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
    lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "进入");
    if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl, sh_get_font_zh(), 0);
    lv_obj_set_style_text_color(lbl, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_center(lbl);
  }
}
