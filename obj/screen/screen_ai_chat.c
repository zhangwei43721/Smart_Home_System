#include "obj/Include/screen_ai_chat.h"
#include "obj/Include/screens_common.h"
#include "obj/Include/config.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "obj/AI/include/data_chat.h"
#include "obj/AI/include/openai.h"

// openai_chat_with_history is implemented in obj/AI/src/chat.c but has no public header
extern char* openai_chat_with_history(ChatNode* history_head, const char* model);

static lv_obj_t* g_msg_cont = NULL;
static lv_obj_t* g_input = NULL;
static bool g_ai_inited = false;
static lv_obj_t* g_btn_send = NULL;
static lv_obj_t* g_actions_row = NULL;
static lv_obj_t* g_back_btn = NULL;

// 导航返回到首页（恢复底栏）
extern void demo_dashboard(void);

static void append_user_bubble(const char* text) {
  if (!g_msg_cont || !text) return;
  lv_obj_t* row = lv_obj_create(g_msg_cont);
  lv_obj_remove_style_all(row);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(row, 6, 0);
  lv_obj_set_style_pad_column(row, 8, 0);

  lv_obj_t* bubble = lv_obj_create(row);
  lv_obj_add_style(bubble, sh_style_card(), 0);
  lv_obj_set_style_bg_color(bubble, lv_palette_lighten(LV_PALETTE_BLUE, 4), 0);
  lv_obj_set_width(bubble, LV_SIZE_CONTENT);
  lv_obj_set_style_max_width(bubble, 100, 10);
  lv_obj_set_style_pad_all(bubble, 10, 0);
  lv_obj_set_flex_grow(bubble, 0);

  lv_obj_t* lbl = lv_label_create(bubble);
  lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
  lv_label_set_text(lbl, text);
  lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
}

static lv_obj_t* append_ai_text_placeholder(const char* text) {
  if (!g_msg_cont || !text) return NULL;
  lv_obj_t* lbl = lv_label_create(g_msg_cont);
  lv_obj_add_style(lbl, sh_style_text_zh(), 0);
  lv_obj_set_width(lbl, LV_PCT(100));
  lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, 0);
  lv_label_set_text(lbl, text);
  return lbl;
}

static void append_ai_text_final(lv_obj_t* lbl, const char* text) {
  if (!lbl || !text) return;
  lv_label_set_text(lbl, text);
}

static void render_history(void) {
  if (!chat_head || list_empty(&chat_head->list)) return;
  ChatNode* p;
  list_for_each_entry(p, &chat_head->list, list) {
    if (strcmp(p->data.role, "user") == 0) {
      append_user_bubble(p->data.content);
    } else {
      append_ai_text_placeholder(p->data.content);
    }
  }
}

static void set_sending_state(bool sending) {
  if (g_btn_send) {
    if (sending) lv_obj_add_state(g_btn_send, LV_STATE_DISABLED); else lv_obj_clear_state(g_btn_send, LV_STATE_DISABLED);
  }
  if (g_input) {
    if (sending) lv_obj_add_state(g_input, LV_STATE_DISABLED); else lv_obj_clear_state(g_input, LV_STATE_DISABLED);
  }
}

static void on_new_chat(lv_event_t* e) {
  LV_UNUSED(e);
  lv_obj_clean(g_msg_cont);
  data_chat_free();
  data_chat_init_list();
}

static void on_go_back(lv_event_t* e) {
  LV_UNUSED(e);
  demo_dashboard();
}

typedef struct {
  lv_obj_t* placeholder_lbl;
  char* resp;
} ai_result_t;

static void ui_apply_ai_result(void* p) {
  ai_result_t* r = (ai_result_t*)p;
  if (r) {
    if (r->resp) {
      append_ai_text_final(r->placeholder_lbl, r->resp);
      data_chat_add("assistant", r->resp);
      free(r->resp);
    } else {
      append_ai_text_final(r->placeholder_lbl, "请求失败");
    }
    set_sending_state(false);
    lv_obj_scroll_to_y(g_msg_cont, lv_obj_get_scroll_top(g_msg_cont) + 10000, LV_ANIM_ON);
    free(r);
  }
}

static void* worker_thread(void* arg) {
  lv_obj_t* placeholder = (lv_obj_t*)arg;
  char* resp = openai_chat_with_history(chat_head, "deepseek-chat");
  ai_result_t* r = (ai_result_t*)malloc(sizeof(ai_result_t));
  if (!r) {
    ai_result_t tmp = {placeholder, NULL};
    lv_async_call(ui_apply_ai_result, &tmp); // fallback unlikely
    return NULL;
  }
  r->placeholder_lbl = placeholder;
  r->resp = resp;
  lv_async_call(ui_apply_ai_result, r);
  return NULL;
}

static void on_send(lv_event_t* e) {
  LV_UNUSED(e);
  if (!g_input) return;
  const char* txt = lv_textarea_get_text(g_input);
  if (!txt || strlen(txt) == 0) return;

  append_user_bubble(txt);
  data_chat_add("user", txt);
  lv_textarea_set_text(g_input, "");

  if (!g_ai_inited) {
    if (strlen(OPENAI_API_KEY) > 0) {
      openai_init(OPENAI_API_KEY);
      g_ai_inited = true;
    } else {
      append_ai_text_placeholder("请先在设置中配置 API Key");
      return;
    }
  }

  set_sending_state(true);
  lv_obj_t* ph = append_ai_text_placeholder("思考中...");
  pthread_t tid;
  pthread_create(&tid, NULL, worker_thread, ph);
  pthread_detach(tid);
}

void screen_ai_chat_build(void) {
  lv_obj_t* scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("AI 对话");

  if (!chat_head) data_chat_init_list();

  lv_obj_t* cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  // 高度扣除底部导航栏高度 56 像素，避免被遮挡
  lv_obj_set_size(cont, 800, 480 - 48);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(cont, 12, 0);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(cont, 12, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

  g_actions_row = lv_obj_create(cont);
  lv_obj_remove_style_all(g_actions_row);
  lv_obj_set_size(g_actions_row, 800 - 24, 36);
  lv_obj_set_flex_flow(g_actions_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(g_actions_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(g_actions_row, LV_OBJ_FLAG_SCROLLABLE);

  // 左侧返回
  g_back_btn = lv_btn_create(g_actions_row);
  lv_obj_add_style(g_back_btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(g_back_btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_size(g_back_btn, 72, 32);
  lv_obj_add_event_cb(g_back_btn, on_go_back, LV_EVENT_CLICKED, NULL);
  {
    lv_obj_t* l = lv_label_create(g_back_btn);
    lv_obj_add_style(l, sh_style_text_zh_semibold(), 0);
    lv_label_set_text(l, "返回");
    lv_obj_center(l);
  }

  // 右侧新对话
  lv_obj_t* btn_new = lv_btn_create(g_actions_row);
  lv_obj_add_style(btn_new, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn_new, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(btn_new, on_new_chat, LV_EVENT_CLICKED, NULL);
  {
    lv_obj_t* l = lv_label_create(btn_new);
    lv_obj_add_style(l, sh_style_text_zh_semibold(), 0);
    lv_label_set_text(l, "新对话");
  }

  lv_obj_t* card_msgs = lv_obj_create(cont);
  lv_obj_add_style(card_msgs, sh_style_card(), 0);
  lv_obj_set_width(card_msgs, 800 - 24);
  lv_obj_set_flex_grow(card_msgs, 1);
  lv_obj_set_style_pad_all(card_msgs, 8, 0);
  lv_obj_set_flex_flow(card_msgs, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card_msgs, 8, 0);
  lv_obj_set_scroll_dir(card_msgs, LV_DIR_VER);
  // 只允许聊天区滚动，其他容器禁止滚动
  lv_obj_clear_flag(card_msgs, LV_OBJ_FLAG_SCROLL_CHAIN);
  g_msg_cont = card_msgs;
  if (!chat_head || list_empty(&chat_head->list)) {
    data_chat_add("assistant", "你好，我是你的智能家居助手\n可以帮你控制灯光、空调，或查询家中环境\n");
  }
  render_history();

  lv_obj_t* input_row = lv_obj_create(cont);
  lv_obj_remove_style_all(input_row);
  lv_obj_set_size(input_row, 800 - 24, 48);
  lv_obj_set_flex_flow(input_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(input_row, 8, 0);
  lv_obj_clear_flag(input_row, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* ta = lv_textarea_create(input_row);
  lv_obj_set_flex_grow(ta, 1);
  lv_obj_set_height(ta, LV_PCT(100));
  lv_textarea_set_one_line(ta, true);
  lv_obj_add_style(ta, sh_style_text_zh(), 0);
  g_input = ta;

  lv_obj_t* btn = lv_btn_create(input_row);
  lv_obj_set_width(btn, 90);
  lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_add_event_cb(btn, on_send, LV_EVENT_CLICKED, NULL);
  {
    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_add_style(lbl, sh_style_text_zh_semibold(), 0);
    lv_label_set_text(lbl, "发送");
    lv_obj_center(lbl);
  }
  g_btn_send = btn;
}

