#include "obj/Include/screen_security.h"
#include "obj/Include/screens_common.h"
#include "obj/data/state_store.h"
#include "obj/Include/Hardware.h"

typedef enum {
  SEC_DISARMED = 0,
  SEC_ARMED_AWAY,
  SEC_ARMED_HOME
} sec_state_t;

static sec_state_t g_sec_state = SEC_DISARMED;

static lv_obj_t * g_btn_disarm = NULL;
static lv_obj_t * g_btn_arm_away = NULL;
static lv_obj_t * g_btn_arm_home = NULL;
static lv_obj_t * g_btn_alarm_now = NULL;
static lv_obj_t * g_lbl_alarm_now = NULL;
static int g_alarm_updating = 0; // 防止程序设置状态时触发递归

// 前置声明，避免隐式声明导致的静态冲突
static void sec_update_alarm_now(int on);
static void on_any_deleted(lv_event_t *e);

static void sec_update_buttons(void) {
  if (!g_btn_disarm || !g_btn_arm_away || !g_btn_arm_home) return;
  lv_obj_clear_state(g_btn_disarm, LV_STATE_CHECKED);
  lv_obj_clear_state(g_btn_arm_away, LV_STATE_CHECKED);
  lv_obj_clear_state(g_btn_arm_home, LV_STATE_CHECKED);
  if (g_sec_state == SEC_DISARMED) lv_obj_add_state(g_btn_disarm, LV_STATE_CHECKED);
  else if (g_sec_state == SEC_ARMED_AWAY) lv_obj_add_state(g_btn_arm_away, LV_STATE_CHECKED);
  else lv_obj_add_state(g_btn_arm_home, LV_STATE_CHECKED);
}

void security_set_alarm_active(int on) {
  control_buzzer(on ? 1 : 0);
  ss_alarm_save(on ? 1 : 0);
  g_alarm_updating = 1;
  sec_update_alarm_now(on ? 1 : 0);
  g_alarm_updating = 0;
}

static void on_disarm(lv_event_t *e) { LV_UNUSED(e); g_sec_state = SEC_DISARMED; ss_security_save((int)g_sec_state); sec_update_buttons(); }
static void on_arm_away(lv_event_t *e) { LV_UNUSED(e); g_sec_state = SEC_ARMED_AWAY; ss_security_save((int)g_sec_state); sec_update_buttons(); }
static void on_arm_home(lv_event_t *e) { LV_UNUSED(e); g_sec_state = SEC_ARMED_HOME; ss_security_save((int)g_sec_state); sec_update_buttons(); }

static void on_alarm_now_changed(lv_event_t *e) {
  if (g_alarm_updating) return;
  lv_obj_t *btn = lv_event_get_target(e);
  int on = lv_obj_has_state(btn, LV_STATE_CHECKED) ? 1 : 0;
  security_set_alarm_active(on);
}

static void sec_update_alarm_now(int on) {
  if (!g_btn_alarm_now) return;
  if (on) {
    lv_obj_add_state(g_btn_alarm_now, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(g_btn_alarm_now, lv_palette_main(LV_PALETTE_RED), LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(g_btn_alarm_now, LV_OPA_COVER, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_color(g_btn_alarm_now, lv_palette_main(LV_PALETTE_RED), LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(g_btn_alarm_now, 18, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(g_btn_alarm_now, 8, LV_STATE_CHECKED);
    lv_obj_set_style_border_color(g_btn_alarm_now, lv_palette_main(LV_PALETTE_RED), LV_STATE_CHECKED);
    lv_obj_set_style_border_width(g_btn_alarm_now, 2, LV_STATE_CHECKED);
    // 按下时稍微加深，符合 Material 的按压反馈
    lv_obj_set_style_bg_color(g_btn_alarm_now, lv_palette_darken(LV_PALETTE_RED, 1), LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_ofs_y(g_btn_alarm_now, 10, LV_STATE_CHECKED | LV_STATE_PRESSED);
    if (g_lbl_alarm_now) lv_obj_set_style_text_color(g_lbl_alarm_now, lv_color_white(), 0);
  } else {
    lv_obj_clear_state(g_btn_alarm_now, LV_STATE_CHECKED);
    if (g_lbl_alarm_now) lv_obj_set_style_text_color(g_lbl_alarm_now, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
  }
}

// 当组件被 LVGL 销毁时，清空对应的静态指针，避免悬空指针导致段错误
static void on_any_deleted(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  if (obj == g_btn_disarm) g_btn_disarm = NULL;
  else if (obj == g_btn_arm_away) g_btn_arm_away = NULL;
  else if (obj == g_btn_arm_home) g_btn_arm_home = NULL;
  else if (obj == g_btn_alarm_now) { g_btn_alarm_now = NULL; }
  else if (obj == g_lbl_alarm_now) { g_lbl_alarm_now = NULL; }
}

// ========== 详情弹窗 ==========
typedef struct { int sensor_index; lv_obj_t *mask; } SecDlgCtx;

static void sec_detail_close(lv_event_t *e) {
  SecDlgCtx *ctx = (SecDlgCtx *)lv_event_get_user_data(e);
  if (ctx && ctx->mask) {
    lv_obj_del(ctx->mask); // 删除蒙层将级联删除子对象
    ctx->mask = NULL;
  }
  if (ctx) lv_mem_free(ctx);
}

static void on_detail_clicked(lv_event_t *e) {
  intptr_t idx = (intptr_t)lv_event_get_user_data(e);
  // 蒙层
  lv_obj_t *mask = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(mask);
  lv_obj_set_size(mask, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(mask, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(mask, LV_OPA_50, 0);

  static const char *names[] = {"门磁","红外","烟雾","水浸","窗磁","震动"};

  // 弹窗
  lv_obj_t *dlg = lv_obj_create(mask);
  lv_obj_remove_style_all(dlg);
  lv_obj_set_size(dlg, 420, 300);
  lv_obj_center(dlg);
  lv_obj_set_style_bg_color(dlg, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(dlg, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(dlg, 12, 0);
  lv_obj_set_style_pad_all(dlg, 12, 0);
  lv_obj_set_flex_flow(dlg, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(dlg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(dlg, 6, 0);
  // Material 风格阴影
  lv_obj_set_style_shadow_width(dlg, 10, 0);
  lv_obj_set_style_shadow_ofs_y(dlg, 6, 0);
  lv_obj_set_style_shadow_color(dlg, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);

  // 标题
  lv_obj_t *title = lv_label_create(dlg);
  lv_obj_add_style(title, sh_style_text_zh_semibold(), 0);
  lv_label_set_text_fmt(title, "%s · 最近事件", names[idx % 6]);
  lv_obj_set_style_text_color(title, lv_palette_darken(LV_PALETTE_GREY, 4), 0);

  // 列表（示例数据，后续接入真实日志）
  for (int i = 0; i < 8; ++i) {
    lv_obj_t *row = lv_label_create(dlg);
    lv_obj_add_style(row, sh_style_text_zh_small(), 0);
    lv_obj_set_style_text_color(row, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    // 交替显示 正常/告警 作为占位
    const char *state = (i % 3 == 1) ? "告警" : "正常";
    lv_label_set_text_fmt(row, "#666666 2025-10-15 20:%02d#  %s", 50 - i, state);
    lv_label_set_recolor(row, true);
  }

  // 关闭按钮（放在白色弹窗内，保证对比度）
  lv_obj_t *btn = lv_btn_create(dlg);
  lv_obj_set_size(btn, 88, 36);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  SecDlgCtx *ctx = (SecDlgCtx *)lv_mem_alloc(sizeof(SecDlgCtx));
  if (ctx) { ctx->sensor_index = (int)idx; ctx->mask = mask; }
  lv_obj_add_event_cb(btn, sec_detail_close, LV_EVENT_CLICKED, ctx);
  lv_obj_add_event_cb(mask, sec_detail_close, LV_EVENT_CLICKED, ctx);
  lv_obj_t *lbl = lv_label_create(btn);
  lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
  lv_label_set_text(lbl, "关闭");
  lv_obj_set_style_text_color(lbl, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
  lv_obj_center(lbl);
}

void screen_security_build(void) {
  sh_init_styles_once();
  g_sec_state = (sec_state_t)ss_security_load();
  int alarm_on_initial = ss_alarm_load();

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
  lv_obj_set_style_pad_column(cont, 12, 0);

  // 顶部：安防整体控制按钮行
  lv_obj_t * row_ctrl = lv_obj_create(cont);
  lv_obj_remove_style_all(row_ctrl);
  lv_obj_set_size(row_ctrl, 800 - 24, 50);
  lv_obj_set_flex_flow(row_ctrl, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row_ctrl, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row_ctrl, 12, 0);

  g_btn_disarm = lv_btn_create(row_ctrl);
  lv_obj_set_size(g_btn_disarm, 120, 40);
  lv_obj_add_style(g_btn_disarm, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_btn_disarm, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(g_btn_disarm, on_disarm, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(g_btn_disarm, on_any_deleted, LV_EVENT_DELETE, NULL);
  lv_obj_t * lbl0 = lv_label_create(g_btn_disarm); lv_label_set_text(lbl0, "撤防"); if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl0, sh_get_font_zh(), 0); lv_obj_center(lbl0);
  lv_obj_add_event_cb(lbl0, on_any_deleted, LV_EVENT_DELETE, NULL);

  g_btn_arm_away = lv_btn_create(row_ctrl);
  lv_obj_set_size(g_btn_arm_away, 120, 40);
  lv_obj_add_style(g_btn_arm_away, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_btn_arm_away, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(g_btn_arm_away, on_arm_away, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(g_btn_arm_away, on_any_deleted, LV_EVENT_DELETE, NULL);
  lv_obj_t * lbl1 = lv_label_create(g_btn_arm_away); lv_label_set_text(lbl1, "一键布防"); if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl1, sh_get_font_zh(), 0); lv_obj_center(lbl1);
  lv_obj_add_event_cb(lbl1, on_any_deleted, LV_EVENT_DELETE, NULL);

  g_btn_arm_home = lv_btn_create(row_ctrl);
  lv_obj_set_size(g_btn_arm_home, 120, 40);
  lv_obj_add_style(g_btn_arm_home, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_btn_arm_home, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(g_btn_arm_home, on_arm_home, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(g_btn_arm_home, on_any_deleted, LV_EVENT_DELETE, NULL);
  lv_obj_t * lbl2 = lv_label_create(g_btn_arm_home); lv_label_set_text(lbl2, "在家布防"); if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl2, sh_get_font_zh(), 0); lv_obj_center(lbl2);
  lv_obj_add_event_cb(lbl2, on_any_deleted, LV_EVENT_DELETE, NULL);

  // 新增：立即报警按钮（位于在家布防右侧）
  g_btn_alarm_now = lv_btn_create(row_ctrl);
  lv_obj_set_size(g_btn_alarm_now, 120, 40);
  lv_obj_add_style(g_btn_alarm_now, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_btn_alarm_now, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  // 允许被置为 CHECKED，以便保持激活态样式
  lv_obj_add_flag(g_btn_alarm_now, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(g_btn_alarm_now, on_alarm_now_changed, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(g_btn_alarm_now, on_any_deleted, LV_EVENT_DELETE, NULL);
  g_lbl_alarm_now = lv_label_create(g_btn_alarm_now); lv_label_set_text(g_lbl_alarm_now, "立即报警"); if (sh_get_font_zh()) lv_obj_set_style_text_font(g_lbl_alarm_now, sh_get_font_zh(), 0); lv_obj_center(g_lbl_alarm_now);
  lv_obj_add_event_cb(g_lbl_alarm_now, on_any_deleted, LV_EVENT_DELETE, NULL);
  // 根据持久化状态恢复按钮与蜂鸣器
  security_set_alarm_active(alarm_on_initial ? 1 : 0);

  sec_update_buttons();

  // 传感器卡片网格
  lv_obj_t * grid = lv_obj_create(cont);
  lv_obj_remove_style_all(grid);
  lv_obj_set_size(grid, 800 - 24, (480 - 48 - 56 - 24) - 50 - 12);
  lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_row(grid, 12, 0);
  lv_obj_set_style_pad_column(grid, 12, 0);

  const char * items[] = {"门磁", "红外", "烟雾", "水浸", "窗磁", "震动"};
  for (int i = 0; i < 6; ++i) {
    lv_obj_t * card = lv_obj_create(grid);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, (800 - 24 - 12) / 2, (480 - 48 - 56 - 12*3) / 2);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, items[i]);
    lv_obj_add_style(title, sh_style_text_zh_semibold(), 0);
    lv_obj_set_style_text_color(title, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 6);

    lv_obj_t * status = lv_label_create(card);
    lv_label_set_text(status, (i % 2 == 0) ? "正常" : "告警");
    lv_obj_add_style(status, sh_style_text_zh_small(), 0);
    lv_obj_set_style_text_color(status, (i % 2 == 0) ? lv_palette_darken(LV_PALETTE_GREY, 3)
                                                     : lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align(status, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t * btn = lv_btn_create(card);
    lv_obj_set_size(btn, 120, 36);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
    lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, on_detail_clicked, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "详情");
    lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
    lv_obj_set_style_text_color(lbl, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_center(lbl);
  }
}
