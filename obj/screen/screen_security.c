#include "obj/Include/screen_security.h"
#include "obj/Include/screens_common.h"
#include "obj/data/state_store.h"

typedef enum {
  SEC_DISARMED = 0,
  SEC_ARMED_AWAY,
  SEC_ARMED_HOME
} sec_state_t;

static sec_state_t g_sec_state = SEC_DISARMED;

static lv_obj_t * g_btn_disarm = NULL;
static lv_obj_t * g_btn_arm_away = NULL;
static lv_obj_t * g_btn_arm_home = NULL;

static void sec_update_buttons(void) {
  if (!g_btn_disarm || !g_btn_arm_away || !g_btn_arm_home) return;
  lv_obj_clear_state(g_btn_disarm, LV_STATE_CHECKED);
  lv_obj_clear_state(g_btn_arm_away, LV_STATE_CHECKED);
  lv_obj_clear_state(g_btn_arm_home, LV_STATE_CHECKED);
  if (g_sec_state == SEC_DISARMED) lv_obj_add_state(g_btn_disarm, LV_STATE_CHECKED);
  else if (g_sec_state == SEC_ARMED_AWAY) lv_obj_add_state(g_btn_arm_away, LV_STATE_CHECKED);
  else lv_obj_add_state(g_btn_arm_home, LV_STATE_CHECKED);
}

static void on_disarm(lv_event_t *e) { LV_UNUSED(e); g_sec_state = SEC_DISARMED; ss_security_save((int)g_sec_state); sec_update_buttons(); }
static void on_arm_away(lv_event_t *e) { LV_UNUSED(e); g_sec_state = SEC_ARMED_AWAY; ss_security_save((int)g_sec_state); sec_update_buttons(); }
static void on_arm_home(lv_event_t *e) { LV_UNUSED(e); g_sec_state = SEC_ARMED_HOME; ss_security_save((int)g_sec_state); sec_update_buttons(); }

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
  lv_obj_t * lbl0 = lv_label_create(g_btn_disarm); lv_label_set_text(lbl0, "撤防"); if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl0, sh_get_font_zh(), 0); lv_obj_center(lbl0);

  g_btn_arm_away = lv_btn_create(row_ctrl);
  lv_obj_set_size(g_btn_arm_away, 120, 40);
  lv_obj_add_style(g_btn_arm_away, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_btn_arm_away, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(g_btn_arm_away, on_arm_away, LV_EVENT_CLICKED, NULL);
  lv_obj_t * lbl1 = lv_label_create(g_btn_arm_away); lv_label_set_text(lbl1, "一键布防"); if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl1, sh_get_font_zh(), 0); lv_obj_center(lbl1);

  g_btn_arm_home = lv_btn_create(row_ctrl);
  lv_obj_set_size(g_btn_arm_home, 120, 40);
  lv_obj_add_style(g_btn_arm_home, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_btn_arm_home, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(g_btn_arm_home, on_arm_home, LV_EVENT_CLICKED, NULL);
  lv_obj_t * lbl2 = lv_label_create(g_btn_arm_home); lv_label_set_text(lbl2, "在家布防"); if (sh_get_font_zh()) lv_obj_set_style_text_font(lbl2, sh_get_font_zh(), 0); lv_obj_center(lbl2);

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
