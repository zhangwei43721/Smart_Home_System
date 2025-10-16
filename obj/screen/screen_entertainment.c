#include "obj/Include/screens_common.h"
#include "obj/Include/screen_entertainment.h"
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

// 外部：从首页返回组件
extern void demo_dashboard(void);
extern void demo_entertainment(void);

// 工具：简单判断文件扩展名
static bool has_ext(const char *name, const char *ext) {
  if (!name || !ext) return false;
  size_t ln = strlen(name), le = strlen(ext);
  if (le > ln) return false;
  const char *p = name + (ln - le);
  for (size_t i = 0; i < le; ++i) {
    if (tolower((unsigned char)p[i]) != tolower((unsigned char)ext[i])) return false;
  }
  return true;
}

static bool is_image_file(const char *name) {
  return has_ext(name, ".png") || has_ext(name, ".jpg") || has_ext(name, ".jpeg") || has_ext(name, ".gif") || has_ext(name, ".sjpg");
}

// ========== 返回处理 ==========
static void on_back_to_entertainment(lv_event_t *e) { LV_UNUSED(e); demo_entertainment(); }
static void on_back_to_dashboard(lv_event_t *e) { LV_UNUSED(e); demo_dashboard(); }

// ========== 子页：图片 ==========
typedef struct { lv_obj_t *img; const char *path; } ImgItemCtx;
static void on_image_item_clicked(lv_event_t *e) {
  ImgItemCtx *ctx = (ImgItemCtx *)lv_event_get_user_data(e);
  if (ctx && ctx->img && ctx->path) {
    LV_LOG_USER("[images] load -> %s", ctx->path);
    lv_img_set_src(ctx->img, ctx->path); // 依赖 LVGL 文件系统与解码器配置
    lv_obj_center(ctx->img);
  }
}

void screen_entertainment_images(void) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  // 顶栏
  sh_create_topbar("图片 Images");

  // 主容器（卡片）
  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 480 - 48 - 24);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 48 + 12);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_all(card, 12, 0);
  lv_obj_set_style_pad_column(card, 12, 0);
  lv_obj_set_style_pad_row(card, 12, 0);

  // 左：文件列表
  lv_obj_t *left = lv_obj_create(card);
  lv_obj_remove_style_all(left);
  lv_obj_set_size(left, 260, LV_PCT(100));
  lv_obj_set_style_bg_color(left, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(left, 1, 0);
  lv_obj_set_style_border_color(left, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
  lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(left, 8, 0);
  lv_obj_set_style_pad_row(left, 6, 0);
  lv_obj_set_scroll_dir(left, LV_DIR_VER);

  // 右：预览
  lv_obj_t *right = lv_obj_create(card);
  lv_obj_remove_style_all(right);
  lv_obj_set_flex_grow(right, 1);
  lv_obj_set_style_bg_color(right, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(right, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(right, 1, 0);
  lv_obj_set_style_border_color(right, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
  lv_obj_set_style_pad_all(right, 8, 0);

  lv_obj_t *img = lv_img_create(right);
  lv_obj_center(img);

  // 读取目录，列出图片（从工作目录下的 ./media/）
  DIR *d = opendir("./media");
  if (d) {
    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
      if (de->d_name[0] == '.') continue;
      if (!is_image_file(de->d_name)) continue;
      LV_LOG_USER("[images] found file: %s", de->d_name);
      lv_obj_t *row = lv_btn_create(left);
      lv_obj_set_size(row, LV_PCT(100), 32);
      lv_obj_add_style(row, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_style(row, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
      lv_obj_set_style_pad_left(row, 8, 0);
      lv_obj_set_style_pad_right(row, 8, 0);
      lv_obj_t *lbl = lv_label_create(row);
      lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
      lv_label_set_text(lbl, de->d_name);
      lv_obj_center(lbl);
      // 为按钮准备事件上下文：包含右侧 img 与路径
      ImgItemCtx *ictx = (ImgItemCtx *)malloc(sizeof(ImgItemCtx));
      if (ictx) {
        ictx->img = img;
        size_t plen = strlen("A:/media/") + strlen(de->d_name) + 1;
        char *full = (char *)malloc(plen);
        if (full) { snprintf(full, plen, "A:/media/%s", de->d_name); }
        ictx->path = full ? full : strdup(de->d_name);
      }
      lv_obj_add_event_cb(row, on_image_item_clicked, LV_EVENT_CLICKED, ictx);
    }
    closedir(d);
  } else {
    lv_obj_t *lbl = lv_label_create(left);
    lv_obj_add_style(lbl, sh_style_text_zh(), 0);
    lv_label_set_text(lbl, "无法打开目录");
  }

  // 图片页返回按钮（最后创建，置顶）
  lv_obj_t * btn_back_img = lv_btn_create(scr);
  lv_obj_set_size(btn_back_img, 50, 50);
  lv_obj_set_style_radius(btn_back_img, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(btn_back_img, LV_ALIGN_BOTTOM_LEFT, 25, -25);
  lv_obj_add_event_cb(btn_back_img, on_back_to_entertainment, LV_EVENT_CLICKED, NULL);
  lv_label_set_text(lv_label_create(btn_back_img), LV_SYMBOL_LEFT);
}

// ========== 子页：音乐 ==========
static void on_music_back(lv_event_t *e) { LV_UNUSED(e); demo_entertainment(); }
static void on_music_play_pause(lv_event_t *e) { LV_UNUSED(e); /* TODO: 占位 */ }
static void on_music_prev(lv_event_t *e) { LV_UNUSED(e); /* TODO: 占位 */ }
static void on_music_next(lv_event_t *e) { LV_UNUSED(e); /* TODO: 占位 */ }

void screen_entertainment_music(void) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("音乐 Music");

  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 480 - 48 - 24);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 48 + 12);
  lv_obj_set_style_pad_all(card, 16, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card, 12, 0);

  // 歌曲信息
  lv_obj_t *title = lv_label_create(card);
  lv_obj_add_style(title, sh_style_text_zh_large(), 0);
  lv_label_set_text(title, "歌曲标题（占位）");

  // 进度条
  lv_obj_t *slider = lv_slider_create(card);
  lv_obj_set_width(slider, LV_PCT(100));

  // 控制按钮行
  lv_obj_t *row = lv_obj_create(card);
  lv_obj_remove_style_all(row);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 12, 0);

  lv_obj_t *btn_prev = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_prev), LV_SYMBOL_PREV);
  lv_obj_add_event_cb(btn_prev, on_music_prev, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_play = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_play), LV_SYMBOL_PLAY);
  lv_obj_add_event_cb(btn_play, on_music_play_pause, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_next = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_next), LV_SYMBOL_NEXT);
  lv_obj_add_event_cb(btn_next, on_music_next, LV_EVENT_CLICKED, NULL);

  // 返回按钮（最后创建，置顶）
  lv_obj_t * btn_back = lv_btn_create(scr);
  lv_obj_set_size(btn_back, 50, 50);
  lv_obj_set_style_radius(btn_back, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 25, -25);
  lv_obj_add_event_cb(btn_back, on_music_back, LV_EVENT_CLICKED, NULL);
  lv_label_set_text(lv_label_create(btn_back), LV_SYMBOL_LEFT);
}

// ========== 子页：视频 ==========
static void on_video_back(lv_event_t *e) { LV_UNUSED(e); demo_entertainment(); }
static void on_video_play_pause(lv_event_t *e) { LV_UNUSED(e); /* TODO: 占位 */ }

void screen_entertainment_video(void) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("视频 Video");

  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 480 - 48 - 24);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 48 + 12);
  lv_obj_set_style_pad_all(card, 16, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card, 12, 0);

  // 视频显示占位（可替换为实际视频画面）
  lv_obj_t *video_area = lv_obj_create(card);
  lv_obj_set_size(video_area, LV_PCT(100), 240);
  lv_obj_set_style_bg_color(video_area, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_bg_opa(video_area, LV_OPA_COVER, 0);

  // 进度条 + 控制
  lv_obj_t *slider = lv_slider_create(card);
  lv_obj_set_width(slider, LV_PCT(100));

  lv_obj_t *row = lv_obj_create(card);
  lv_obj_remove_style_all(row);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 12, 0);

  lv_obj_t *btn_play = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_play), LV_SYMBOL_PLAY);
  lv_obj_add_event_cb(btn_play, on_video_play_pause, LV_EVENT_CLICKED, NULL);

  // 返回按钮（最后创建，置顶）
  lv_obj_t * btn_back = lv_btn_create(scr);
  lv_obj_set_size(btn_back, 50, 50);
  lv_obj_set_style_radius(btn_back, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 25, -25);
  lv_obj_add_event_cb(btn_back, on_video_back, LV_EVENT_CLICKED, NULL);
  lv_label_set_text(lv_label_create(btn_back), LV_SYMBOL_LEFT);
}

// ========== 娱乐主页 ==========
static void on_go_images(lv_event_t *e) { LV_UNUSED(e); screen_entertainment_images(); }
static void on_go_music(lv_event_t *e) { LV_UNUSED(e); screen_entertainment_music(); }
static void on_go_video(lv_event_t *e) { LV_UNUSED(e); screen_entertainment_video(); }

void screen_entertainment_build(void) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("娱乐 Entertainment");

  // 内容容器
  lv_obj_t *cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(cont, 12, 0);

  // 三张卡片：图片/音乐/视频
  lv_obj_t *grid = lv_obj_create(cont);
  lv_obj_remove_style_all(grid);
  lv_obj_set_size(grid, 800 - 24, 480 - 48 - 24);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(grid, 12, 0);
  lv_obj_set_style_pad_row(grid, 12, 0);
  lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  const char *names[3] = {"图片", "音乐", "视频"};
  lv_event_cb_t cbs[3] = { on_go_images, on_go_music, on_go_video };

  for (int i = 0; i < 3; ++i) {
    lv_obj_t *card = lv_obj_create(grid);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, (800 - 24 - 12*2)/3, 320);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *btn = lv_btn_create(card);
    lv_obj_add_style(btn, sh_style_btn_neutral(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, sh_style_btn_neutral_pressed(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_size(btn, LV_PCT(100), LV_PCT(100));
    lv_obj_add_event_cb(btn, cbs[i], LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_add_style(lbl, sh_style_text_zh_large(), 0);
    lv_label_set_text(lbl, names[i]);
    lv_obj_center(lbl);
  }

  // 主选项页不显示返回按钮，返回按钮仅在各子页中显示
}
