#include "obj/Include/screens_common.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
// JoyPixels emoji symbols used by helpers (only names without hyphen)
extern const lv_img_dsc_t sunny;
extern const lv_img_dsc_t cloud;
extern const lv_img_dsc_t rain;
extern const lv_img_dsc_t snow;
extern const lv_img_dsc_t fog;
extern const lv_img_dsc_t thunderstorm;
extern const lv_img_dsc_t wind;
extern const lv_img_dsc_t moon;

static lv_style_t g_style_title;
static lv_style_t g_style_card;
static lv_style_t g_style_nav_text;
static lv_style_t g_style_card_info;
static lv_style_t g_style_text_zh;
static lv_style_t g_style_text_zh_large;
static lv_style_t g_style_text_zh_small;
static lv_style_t g_style_text_zh_semibold;
static lv_style_t g_style_btn_neutral;
static lv_style_t g_style_btn_neutral_pressed;
static bool g_styles_inited_common = false;

static lv_font_t * g_font_zh_common = NULL;
static lv_font_t * g_font_zh_large = NULL;
static lv_font_t * g_font_zh_small = NULL;
static lv_font_t * g_font_zh_semibold = NULL;
static lv_obj_t * g_time_label = NULL;
static lv_timer_t * g_time_timer = NULL;

static void ensure_font_loaded(void) {
  if (!g_font_zh_common) {
    static lv_ft_info_t info24;
    info24.name = "./media/MiSans-Regular.ttf";
    info24.weight = 22;
    info24.style = FT_FONT_STYLE_NORMAL;
    info24.mem = NULL;
    if (lv_ft_font_init(&info24)) g_font_zh_common = info24.font;
  }
  if (!g_font_zh_semibold) {
    static lv_ft_info_t info24b;
    info24b.name = "./media/MiSans-Semibold.ttf";
    info24b.weight = 24;
    info24b.style = FT_FONT_STYLE_NORMAL;
    info24b.mem = NULL;
    if (lv_ft_font_init(&info24b)) g_font_zh_semibold = info24b.font;
  }
  if (!g_font_zh_large) {
    static lv_ft_info_t info48;
    info48.name = "./media/MiSans-Semibold.ttf";
    info48.weight = 36;
    info48.style = FT_FONT_STYLE_NORMAL;
    info48.mem = NULL;
    if (lv_ft_font_init(&info48)) g_font_zh_large = info48.font;
  }
  if (!g_font_zh_small) {
    static lv_ft_info_t info18;
    info18.name = "./media/MiSans-Regular.ttf";
    info18.weight = 18;
    info18.style = FT_FONT_STYLE_NORMAL;
    info18.mem = NULL;
    if (lv_ft_font_init(&info18)) g_font_zh_small = info18.font;
  }
}

void sh_init_styles_once(void) {
  if (g_styles_inited_common) return;
  ensure_font_loaded();

  lv_style_init(&g_style_title);
  lv_style_set_text_font(&g_style_title, g_font_zh_semibold ? g_font_zh_semibold : (g_font_zh_common ? g_font_zh_common : LV_FONT_DEFAULT));
  lv_style_set_text_color(&g_style_title, lv_palette_darken(LV_PALETTE_GREY, 4));

  lv_style_init(&g_style_card);
  lv_style_set_radius(&g_style_card, 10);
  lv_style_set_bg_color(&g_style_card, lv_color_white());
  lv_style_set_bg_opa(&g_style_card, LV_OPA_COVER);
  lv_style_set_pad_all(&g_style_card, 12);
  lv_style_set_shadow_width(&g_style_card, 6);
  lv_style_set_shadow_ofs_y(&g_style_card, 3);
  lv_style_set_shadow_color(&g_style_card, lv_palette_lighten(LV_PALETTE_GREY, 2));
  lv_style_set_shadow_opa(&g_style_card, LV_OPA_30);

  // 信息卡（淡蓝底）
  lv_style_init(&g_style_card_info);
  lv_style_set_radius(&g_style_card_info, 10);
  lv_style_set_bg_color(&g_style_card_info, lv_palette_lighten(LV_PALETTE_BLUE, 4));
  lv_style_set_bg_opa(&g_style_card_info, LV_OPA_COVER);
  lv_style_set_pad_all(&g_style_card_info, 12);
  lv_style_set_shadow_width(&g_style_card_info, 4);
  lv_style_set_shadow_ofs_y(&g_style_card_info, 2);
  lv_style_set_shadow_color(&g_style_card_info, lv_palette_lighten(LV_PALETTE_BLUE, 3));
  lv_style_set_shadow_opa(&g_style_card_info, LV_OPA_30);

  lv_style_init(&g_style_nav_text);
  lv_style_set_text_font(&g_style_nav_text, g_font_zh_semibold ? g_font_zh_semibold : (g_font_zh_common ? g_font_zh_common : LV_FONT_DEFAULT));
  lv_style_set_text_color(&g_style_nav_text, lv_palette_darken(LV_PALETTE_GREY, 3));

  // 中文文本样式（示例：等效于 demo_freetype_text 中的 style）
  lv_style_init(&g_style_text_zh);
  lv_style_set_text_font(&g_style_text_zh, g_font_zh_common ? g_font_zh_common : LV_FONT_DEFAULT);
  lv_style_set_text_align(&g_style_text_zh, LV_TEXT_ALIGN_CENTER);

  // 中文大号文字样式
  lv_style_init(&g_style_text_zh_large);
  lv_style_set_text_font(&g_style_text_zh_large, g_font_zh_large ? g_font_zh_large : (g_font_zh_common ? g_font_zh_common : LV_FONT_DEFAULT));
  lv_style_set_text_align(&g_style_text_zh_large, LV_TEXT_ALIGN_CENTER);

  // 中文小号文字样式
  lv_style_init(&g_style_text_zh_small);
  lv_style_set_text_font(&g_style_text_zh_small, g_font_zh_small ? g_font_zh_small : (g_font_zh_common ? g_font_zh_common : LV_FONT_DEFAULT));
  lv_style_set_text_align(&g_style_text_zh_small, LV_TEXT_ALIGN_CENTER);

  lv_style_init(&g_style_text_zh_semibold);
  lv_style_set_text_font(&g_style_text_zh_semibold, g_font_zh_semibold ? g_font_zh_semibold : (g_font_zh_common ? g_font_zh_common : LV_FONT_DEFAULT));
  lv_style_set_text_align(&g_style_text_zh_semibold, LV_TEXT_ALIGN_CENTER);

  lv_style_init(&g_style_btn_neutral);
  lv_style_set_radius(&g_style_btn_neutral, 10);
  lv_style_set_bg_color(&g_style_btn_neutral, lv_color_white());
  lv_style_set_bg_opa(&g_style_btn_neutral, LV_OPA_COVER);
  lv_style_set_border_width(&g_style_btn_neutral, 1);
  lv_style_set_border_color(&g_style_btn_neutral, lv_palette_lighten(LV_PALETTE_GREY, 2));
  lv_style_set_text_color(&g_style_btn_neutral, lv_palette_darken(LV_PALETTE_GREY, 3));

  lv_style_init(&g_style_btn_neutral_pressed);
  lv_style_set_bg_color(&g_style_btn_neutral_pressed, lv_palette_lighten(LV_PALETTE_GREY, 4));
  lv_style_set_text_color(&g_style_btn_neutral_pressed, lv_palette_darken(LV_PALETTE_GREY, 4));

  g_styles_inited_common = true;
}

lv_font_t * sh_get_font_zh(void) { return g_font_zh_common; }
lv_font_t * sh_get_font_zh_large(void) { return g_font_zh_large ? g_font_zh_large : (g_font_zh_common ? g_font_zh_common : (lv_font_t*)LV_FONT_DEFAULT); }
lv_font_t * sh_get_font_zh_small(void) { return g_font_zh_small ? g_font_zh_small : (g_font_zh_common ? g_font_zh_common : (lv_font_t*)LV_FONT_DEFAULT); }
lv_font_t * sh_get_font_zh_semibold(void) { return g_font_zh_semibold ? g_font_zh_semibold : (g_font_zh_common ? g_font_zh_common : (lv_font_t*)LV_FONT_DEFAULT); }

lv_style_t * sh_style_title(void) { return &g_style_title; }
lv_style_t * sh_style_card(void) { return &g_style_card; }
lv_style_t * sh_style_card_info(void) { return &g_style_card_info; }
lv_style_t * sh_style_nav_text(void) { return &g_style_nav_text; }
lv_style_t * sh_style_text_zh(void) { return &g_style_text_zh; }
lv_style_t * sh_style_text_zh_large(void) { return &g_style_text_zh_large; }
lv_style_t * sh_style_text_zh_small(void) { return &g_style_text_zh_small; }
lv_style_t * sh_style_text_zh_semibold(void) { return &g_style_text_zh_semibold; }
lv_style_t * sh_style_btn_neutral(void) { return &g_style_btn_neutral; }
lv_style_t * sh_style_btn_neutral_pressed(void) { return &g_style_btn_neutral_pressed; }

static void update_time_cb(lv_timer_t * timer) {
  LV_UNUSED(timer);
  if (!g_time_label) return;
  time_t now = time(NULL);
  struct tm * lt = localtime(&now);
  if (!lt) return;
  static char buf[32];
  strftime(buf, sizeof(buf), "%H:%M:%S", lt);
  lv_label_set_text(g_time_label, buf);
}

lv_obj_t * sh_create_topbar(const char * title) {
  lv_obj_t * scr = lv_scr_act();
  lv_obj_t * bar = lv_obj_create(scr);
  lv_obj_remove_style_all(bar);
  lv_obj_set_size(bar, 800, 48);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(bar, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(bar, 1, 0);
  lv_obj_set_style_border_color(bar, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_shadow_width(bar, 8, 0);
  lv_obj_set_style_shadow_ofs_y(bar, 4, 0);
  lv_obj_set_style_shadow_color(bar, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_shadow_opa(bar, LV_OPA_30, 0);

  lv_obj_t * lbl = lv_label_create(bar);
  lv_obj_add_style(lbl, &g_style_title, 0);
  lv_label_set_text(lbl, title);
  lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 16, 0);

  // 右上角时间
  g_time_label = lv_label_create(bar);
  lv_obj_add_style(g_time_label, &g_style_title, 0);
  lv_obj_align(g_time_label, LV_ALIGN_RIGHT_MID, -12, 0);
  if (g_time_timer) {
    lv_timer_del(g_time_timer);
    g_time_timer = NULL;
  }
  update_time_cb(NULL);
  g_time_timer = lv_timer_create(update_time_cb, 1000, NULL);
  return bar;
}

const lv_img_dsc_t * sh_weather_icon(const char * code) {
  if (!code) return &cloud;
  // normalize simple keywords
  if (!strcmp(code, "sunny") || !strcmp(code, "clear")) return &sunny;
  if (!strcmp(code, "cloud") || !strcmp(code, "cloudy")) return &cloud;
  if (!strcmp(code, "rain") || !strcmp(code, "showers")) return &rain;
  if (!strcmp(code, "snow")) return &snow;
  if (!strcmp(code, "fog") || !strcmp(code, "mist") || !strcmp(code, "haze")) return &fog;
  if (!strcmp(code, "thunder") || !strcmp(code, "thunderstorm") || !strcmp(code, "storm")) return &thunderstorm;
  if (!strcmp(code, "wind") || !strcmp(code, "windy")) return &wind;
  if (!strcmp(code, "night") || !strcmp(code, "moon")) return &moon;
  // numeric code from Seniverse
  if (code[0] >= '0' && code[0] <= '9') {
    int v = atoi(code);
    if (v == 0) return &sunny;           // 晴
    if (v >= 1 && v <= 4) return &cloud; // 多云/阴
    if (v >= 5 && v <= 8) return &fog;   // 雾/霾
    if (v >= 9 && v <= 18) return &rain; // 各类雨
    if (v >= 19 && v <= 25) return &snow;// 各类雪
    if (v >= 26 && v <= 29) return &wind;// 风
    if (v >= 30 && v <= 31) return &thunderstorm; // 雷暴
  }
  // default
  return &cloud;
}
