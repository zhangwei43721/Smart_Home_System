#include "obj/Include/screen_dashboard.h"
#include "obj/http/weather.h"
#include "obj/Include/screen_ac.h"
#include "obj/Include/config.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// JoyPixels emoji C 数组（仅使用无连字符的符号名，避免编译问题）
extern const lv_img_dsc_t cloud;
extern const lv_img_dsc_t bolt;
extern const lv_img_dsc_t thermometer;
extern const lv_img_dsc_t home;
extern const lv_img_dsc_t door;
extern const lv_img_dsc_t film;

// 外部导航函数（在 screens_home.c 提供）
extern void demo_lighting(void);
extern void demo_security(void);
extern void demo_energy(void);
extern void demo_dashboard(void);

typedef struct {
  lv_obj_t *icon;
  lv_obj_t *label;
  char location[64];
  char key[64];
} WeatherUpdater;

typedef struct {
  WeatherUpdater *wu;
  sh_weather_now_t wn;
} WeatherApplyCtx;

static void apply_weather_cb(void *p) {
  WeatherApplyCtx *ctx = (WeatherApplyCtx *)p;
  if (!ctx || !ctx->wu) return;
  const lv_img_dsc_t *ic = sh_weather_icon(ctx->wn.code);
  if (ic) lv_img_set_src(ctx->wu->icon, ic);
  char buf[128];
  snprintf(buf, sizeof(buf), "%s · %s %s℃",
           ctx->wn.location[0]?ctx->wn.location:"-",
           ctx->wn.text[0]?ctx->wn.text:"-",
           ctx->wn.temperature[0]?ctx->wn.temperature:"-");
  lv_label_set_text(ctx->wu->label, buf);
  free(ctx);
}

// 天气更新线程函数
static void *weather_thread(void *arg) {
  WeatherUpdater *wu = (WeatherUpdater *)arg;
  while (1) {
    sh_weather_now_t wn; char err[128];
    if (sh_weather_fetch_now(wu->location, wu->key, &wn, err, sizeof(err)) == 0) {
      WeatherApplyCtx *ctx = (WeatherApplyCtx *)malloc(sizeof(WeatherApplyCtx));
      if (ctx) { ctx->wu = wu; ctx->wn = wn; lv_async_call(apply_weather_cb, ctx); }
    }
    sleep(3600); // 每小时更新一次
  }
  return NULL;
}

static int s_ac_temp = 24;
typedef struct { lv_obj_t *title; lv_obj_t *temp; } AcCtx;
static AcCtx s_ac_ctx;

static void ac_update_labels(lv_obj_t *title_label, lv_obj_t *temp_label) {
  if (title_label) {
    char t[32]; snprintf(t, sizeof(t), "空调 · 客厅 %d℃", s_ac_temp);
    lv_label_set_text(title_label, t);
  }
  if (temp_label) {
    char t2[16]; snprintf(t2, sizeof(t2), "%d℃", s_ac_temp);
    lv_label_set_text(temp_label, t2);
  }
}

static void ac_dec_event_cb(lv_event_t * e) {
  LV_UNUSED(e);
  if (s_ac_temp > 16) s_ac_temp--;
  ac_update_labels(s_ac_ctx.title, s_ac_ctx.temp);
}

static void ac_inc_event_cb(lv_event_t * e) {
  LV_UNUSED(e);
  if (s_ac_temp < 30) s_ac_temp++;
  ac_update_labels(s_ac_ctx.title, s_ac_ctx.temp);
}

// 点击整卡：跳转到空调详情页
static void on_card_climate_clicked(lv_event_t * e) {
  LV_UNUSED(e);
  screen_ac_show();
}

// 点击整卡：跳转到能耗页
static void on_card_energy_clicked(lv_event_t * e) {
  LV_UNUSED(e);
  demo_energy();
}

// 点击整卡：跳转到照明页
static void on_card_lighting_clicked(lv_event_t * e) {
  LV_UNUSED(e);
  demo_lighting();
}

// 点击整卡：跳转到安防页
static void on_card_security_clicked(lv_event_t * e) {
  LV_UNUSED(e);
  demo_security();
}

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
  lv_obj_clear_flag(card_weather, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t * w_icon = lv_img_create(card_weather);
  lv_img_set_src(w_icon, sh_weather_icon("cloud"));
  lv_img_set_zoom(w_icon, 114);
  lv_obj_align(w_icon, LV_ALIGN_LEFT_MID, 8, 0);

  lv_obj_t * w_city = lv_label_create(card_weather);
  lv_obj_add_style(w_city, sh_style_text_zh(), 0);
  lv_label_set_text(w_city, "上海 · 多云 26℃ · 空气优");
  lv_obj_align_to(w_city, w_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  {
    sh_weather_now_t wn; char werr[128];
    if (sh_weather_fetch_now(WEATHER_CITY, WEATHER_API_KEY, &wn, werr, sizeof(werr)) == 0) {
      const lv_img_dsc_t *ic = sh_weather_icon(wn.code);
      if (ic) lv_img_set_src(w_icon, ic);
      char buf[128];
      snprintf(buf, sizeof(buf), "%s · %s %s℃", wn.location[0]?wn.location:"-", wn.text[0]?wn.text:"-", wn.temperature[0]?wn.temperature:"-");
      lv_label_set_text(w_city, buf);
    }
  }

  // 启动天气后台线程（每小时更新一次）
  WeatherUpdater *wu = (WeatherUpdater *)malloc(sizeof(WeatherUpdater));
  if (wu) {
    wu->icon = w_icon; wu->label = w_city;
    snprintf(wu->location, sizeof(wu->location), "%s", WEATHER_CITY);
    snprintf(wu->key, sizeof(wu->key), "%s", WEATHER_API_KEY);
    pthread_t th; pthread_create(&th, NULL, weather_thread, wu); pthread_detach(th);
  }

  // 能耗概览（宽卡）
  lv_obj_t * card_energy = lv_obj_create(list);
  lv_obj_add_style(card_energy, sh_style_card(), 0);
  lv_obj_set_size(card_energy, 800 - 24, 90);
  lv_obj_clear_flag(card_energy, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t * e_icon = lv_img_create(card_energy);
  lv_img_set_src(e_icon, &bolt);
  lv_img_set_zoom(e_icon, 114);
  lv_obj_align(e_icon, LV_ALIGN_LEFT_MID, 8, 0);

  lv_obj_t * lbl_e = lv_label_create(card_energy);
  lv_label_set_text(lbl_e, "今日用电 3.6kWh · 本月 62kWh");
  lv_obj_add_style(lbl_e, sh_style_text_zh(), 0);
  lv_obj_set_style_text_color(lbl_e, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_align_to(lbl_e, e_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  // 预留：整卡点击可跳转能耗页
  lv_obj_add_flag(card_energy, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(card_energy, on_card_energy_clicked, LV_EVENT_CLICKED, NULL);

  // 次行：空调温控卡（半宽）
  lv_obj_t * card_climate = lv_obj_create(list);
  lv_obj_add_style(card_climate, sh_style_card(), 0);
  lv_obj_set_size(card_climate, (800 - 24 - 12) / 2, 140);
  lv_obj_clear_flag(card_climate, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t * c_icon = lv_img_create(card_climate);
  lv_img_set_src(c_icon, &thermometer);
  lv_img_set_zoom(c_icon, 114);
  lv_obj_align(c_icon, LV_ALIGN_TOP_LEFT, 8, 4);

  lv_obj_t * lbl_c = lv_label_create(card_climate);
  lv_label_set_text(lbl_c, "空调 · 客厅 24℃");
  lv_obj_add_style(lbl_c, sh_style_text_zh(), 0);
  lv_obj_set_style_text_color(lbl_c, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_align_to(lbl_c, c_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  // 开关
  lv_obj_t * sw = lv_switch_create(card_climate);
  lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -8, 4);

  lv_obj_t * row_c = lv_obj_create(card_climate);
  lv_obj_remove_style_all(row_c);
  lv_obj_set_size(row_c, ((800 - 24 - 12) / 2) - 16, 40);
  lv_obj_align(row_c, LV_ALIGN_BOTTOM_LEFT, 8, -8);
  lv_obj_set_flex_flow(row_c, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row_c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row_c, 8, 0);
  lv_obj_t * btn_c_dec = lv_btn_create(row_c);
  lv_obj_add_style(btn_c_dec, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_c_dec, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_size(btn_c_dec, 44, 36);
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
  lv_obj_set_size(btn_c_inc, 44, 36);
  lv_obj_t * lbi = lv_label_create(btn_c_inc);
  lv_label_set_text(lbi, "+");
  lv_obj_center(lbi);

  // 设置加减事件：更新温度显示
  s_ac_ctx.title = lbl_c; s_ac_ctx.temp = temp;
  lv_obj_add_event_cb(btn_c_dec, ac_dec_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(btn_c_inc, ac_inc_event_cb, LV_EVENT_CLICKED, NULL);

  // 初始同步
  ac_update_labels(lbl_c, temp);
  // 整卡点击：进入空调详情页
  lv_obj_add_flag(card_climate, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(card_climate, on_card_climate_clicked, LV_EVENT_CLICKED, NULL);

  // 快捷场景（半宽卡，与空调并排）
  lv_obj_t * card_scene = lv_obj_create(list);
  lv_obj_add_style(card_scene, sh_style_card(), 0);
  lv_obj_set_size(card_scene, (800 - 24 - 12) / 2, 140);
  lv_obj_clear_flag(card_scene, LV_OBJ_FLAG_SCROLLABLE);
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
  lv_obj_set_flex_align(row_s, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row_s, 8, 0);
  const char * qscenes[] = {"到家", "离家", "观影"};
  for (int qi = 0; qi < 3; ++qi) {
    lv_obj_t * b = lv_btn_create(row_s);
    lv_obj_add_style(b, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(b, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_size(b, 64, 36);
    
    lv_obj_t * lb = lv_label_create(b);
    lv_obj_add_style(lb, sh_style_text_zh(), 0);
    lv_label_set_text(lb, qscenes[qi]);
    lv_obj_center(lb);
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

    // 整卡可点击导航
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    if (i == 0) {
      lv_obj_add_event_cb(card, on_card_lighting_clicked, LV_EVENT_CLICKED, NULL);
    } else if (i == 1) {
      lv_obj_add_event_cb(card, on_card_security_clicked, LV_EVENT_CLICKED, NULL);
    }
  }
}
