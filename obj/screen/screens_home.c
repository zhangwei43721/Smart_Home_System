
#include "obj/Include/screens_common.h"
#include "obj/Include/screen_dashboard.h"
#include "obj/Include/screen_lighting.h"
#include "obj/Include/screen_security.h"
#include "obj/Include/screen_energy.h"
#include "obj/Include/screen_entertainment.h"

// 仅在本文件保留底栏相关样式
static lv_style_t style_nav_text;
static lv_style_t style_nav_btn;
static lv_style_t style_nav_btn_checked;
static bool g_nav_inited = false;

static void init_styles_once(void) {
  if (g_nav_inited) return;
  sh_init_styles_once();
  lv_style_init(&style_nav_text);
  lv_style_set_text_font(&style_nav_text, sh_get_font_zh() ? sh_get_font_zh() : LV_FONT_DEFAULT);
  lv_style_set_text_color(&style_nav_text, lv_palette_darken(LV_PALETTE_GREY, 3));

  lv_style_init(&style_nav_btn);
  lv_style_set_radius(&style_nav_btn, 16);
  lv_style_set_bg_color(&style_nav_btn, lv_color_white());
  lv_style_set_bg_opa(&style_nav_btn, LV_OPA_COVER);
  lv_style_set_border_width(&style_nav_btn, 1);
  lv_style_set_border_color(&style_nav_btn, lv_palette_lighten(LV_PALETTE_GREY, 3));

  lv_style_init(&style_nav_btn_checked);
  lv_style_set_bg_color(&style_nav_btn_checked, lv_palette_lighten(LV_PALETTE_GREY, 4));
  lv_style_set_border_color(&style_nav_btn_checked, lv_palette_lighten(LV_PALETTE_GREY, 2));
  g_nav_inited = true;
}

// 清空当前屏幕内容
static void clear_screen(void) {
  lv_obj_t * scr = lv_scr_act();
  lv_obj_clean(scr);
}

// 底部导航按钮事件前向声明
static void on_nav_to_dashboard(lv_event_t * e);
static void on_nav_to_lighting(lv_event_t * e);
static void on_nav_to_security(lv_event_t * e);
static void on_nav_to_energy(lv_event_t * e);
static void on_nav_to_entertainment(lv_event_t * e);

// 创建底部导航栏（三个 Tab：概览/照明/安防）
static void create_bottom_nav(const char *active) {
  lv_obj_t * scr = lv_scr_act();
  lv_obj_t * nav = lv_obj_create(scr);
  lv_obj_remove_style_all(nav);
  lv_obj_set_size(nav, 800, 56);
  lv_obj_align(nav, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(nav, lv_color_white(), 0); // 白色底栏
  lv_obj_set_style_bg_opa(nav, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(nav, 1, 0);
  lv_obj_set_style_border_color(nav, lv_palette_lighten(LV_PALETTE_GREY, 3), 0); // 顶部分隔线
  lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(nav, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // 概览按钮
  lv_obj_t * btn_dash = lv_btn_create(nav);
  lv_obj_set_size(btn_dash, 120, 40);
  lv_obj_add_style(btn_dash, &style_nav_btn, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_dash, &style_nav_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(btn_dash, on_nav_to_dashboard, LV_EVENT_CLICKED, NULL);
  if (active && strcmp(active, "dashboard")==0) {
    lv_obj_add_state(btn_dash, LV_STATE_CHECKED);
  }
  lv_obj_t * lbl_dash = lv_label_create(btn_dash);
  lv_label_set_text(lbl_dash, "概览");
  lv_obj_add_style(lbl_dash, &style_nav_text, 0);
  lv_obj_center(lbl_dash);

  // 照明按钮
  lv_obj_t * btn_light = lv_btn_create(nav);
  lv_obj_set_size(btn_light, 120, 40);
  lv_obj_add_style(btn_light, &style_nav_btn, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_light, &style_nav_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(btn_light, on_nav_to_lighting, LV_EVENT_CLICKED, NULL);
  if (active && strcmp(active, "lighting")==0) {
    lv_obj_add_state(btn_light, LV_STATE_CHECKED);
  }
  lv_obj_t * lbl_light = lv_label_create(btn_light);
  lv_label_set_text(lbl_light, "照明");
  lv_obj_add_style(lbl_light, &style_nav_text, 0);
  lv_obj_center(lbl_light);

  // 安防按钮
  lv_obj_t * btn_sec = lv_btn_create(nav);
  lv_obj_set_size(btn_sec, 120, 40);
  lv_obj_add_style(btn_sec, &style_nav_btn, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_sec, &style_nav_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(btn_sec, on_nav_to_security, LV_EVENT_CLICKED, NULL);
  if (active && strcmp(active, "security")==0) {
    lv_obj_add_state(btn_sec, LV_STATE_CHECKED);
  }
  lv_obj_t * lbl_sec = lv_label_create(btn_sec);
  lv_label_set_text(lbl_sec, "安防");
  lv_obj_add_style(lbl_sec, &style_nav_text, 0);
  lv_obj_center(lbl_sec);

  // 娱乐按钮
  lv_obj_t * btn_ent = lv_btn_create(nav);
  lv_obj_set_size(btn_ent, 120, 40);
  lv_obj_add_style(btn_ent, &style_nav_btn, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_ent, &style_nav_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(btn_ent, on_nav_to_entertainment, LV_EVENT_CLICKED, NULL);
  if (active && strcmp(active, "entertainment")==0) {
    lv_obj_add_state(btn_ent, LV_STATE_CHECKED);
  }
  lv_obj_t * lbl_ent = lv_label_create(btn_ent);
  lv_label_set_text(lbl_ent, "娱乐");
  lv_obj_add_style(lbl_ent, &style_nav_text, 0);
  lv_obj_center(lbl_ent);

  LV_UNUSED(active);
}

// 顶部标题栏
// 顶栏由公共模块提供
static inline lv_obj_t * create_topbar(const char * title) { return sh_create_topbar(title); }

// 概览屏内容交由独立模块
static void build_dashboard_content(void) { screen_dashboard_build(); }

// 照明屏内容交由独立模块
static void build_lighting_content(void) { screen_lighting_build(); }

// 安防屏内容交由独立模块
static void build_security_content(void) { screen_security_build(); }

// 能耗屏内容交由独立模块
static void build_energy_content(void) { screen_energy_build(); }


// 事件：切到概览
static void on_nav_to_dashboard(lv_event_t * e) {
  LV_UNUSED(e);
  clear_screen();
  init_styles_once();
  create_topbar("概览 Dashboard");
  build_dashboard_content();
  create_bottom_nav("dashboard");
}

// 事件：切到照明
static void on_nav_to_lighting(lv_event_t * e) {
  LV_UNUSED(e);
  clear_screen();
  init_styles_once();
  create_topbar("照明 Lighting");
  build_lighting_content();
  create_bottom_nav("lighting");
}

// 事件：切到安防
static void on_nav_to_security(lv_event_t * e) {
  LV_UNUSED(e);
  clear_screen();
  init_styles_once();
  create_topbar("安防 Security");
  build_security_content();
  create_bottom_nav("security");
}

// 事件：切到能耗
static void on_nav_to_energy(lv_event_t * e) {
  LV_UNUSED(e);
  clear_screen();
  init_styles_once();
  create_topbar("能耗 Energy");
  build_energy_content();
  create_bottom_nav("energy");
}

// 事件：切到娱乐
static void on_nav_to_entertainment(lv_event_t * e) {
  LV_UNUSED(e);
  clear_screen();
  init_styles_once();
  create_topbar("娱乐 Entertainment");
  screen_entertainment_build();
  create_bottom_nav("entertainment");
}


// 对外导出：加载概览屏
void demo_dashboard(void) {
  on_nav_to_dashboard(NULL);
}

// 对外导出：加载照明屏
void demo_lighting(void) {
  on_nav_to_lighting(NULL);
}

// 对外导出：加载安防屏
void demo_security(void) {
  on_nav_to_security(NULL);
}

// 对外导出：加载能耗屏
void demo_energy(void) {
  on_nav_to_energy(NULL);
}

// 对外导出：加载娱乐屏
void demo_entertainment(void) {
  on_nav_to_entertainment(NULL);
}
