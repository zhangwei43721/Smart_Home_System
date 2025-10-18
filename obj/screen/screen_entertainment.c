#include "obj/Include/screen_entertainment.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "obj/Include/screens_common.h"

// 外部：从首页返回组件
extern void demo_dashboard(void);
extern void demo_entertainment(void);

// 子页返回：回到娱乐主页并恢复底栏
static void on_back_to_entertainment(lv_event_t* e) {
  LV_UNUSED(e);
  demo_entertainment();
}

// ================= 全局变量和工具函数 =================

// 用于存储音乐和视频文件列表
static char** g_music_files = NULL;
static int g_music_count = 0;
static int g_current_music_index = -1;

static char** g_video_files = NULL;
static int g_video_count = 0;
static int g_current_video_index = -1;

// 当前播放状态
static bool g_is_music_playing = false;
static bool g_is_video_playing = false;
static bool g_is_video_paused = false;

// 简单判断文件扩展名
static bool has_ext(const char* name, const char* ext) {
  if (!name || !ext) return false;
  size_t ln = strlen(name), le = strlen(ext);
  if (le > ln) return false;
  const char* p = name + (ln - le);
  for (size_t i = 0; i < le; ++i) {
    if (tolower((unsigned char)p[i]) != tolower((unsigned char)ext[i]))
      return false;
  }
  return true;
}

static bool is_image_file(const char* name) {
  return has_ext(name, ".png") || has_ext(name, ".jpg") ||
         has_ext(name, ".jpeg") || has_ext(name, ".gif") ||
         has_ext(name, ".sjpg");
}

static bool is_music_file(const char* name) { return has_ext(name, ".mp3"); }

static bool is_video_file(const char* name) {
  return has_ext(name, ".avi") || has_ext(name, ".mp4") ||
         has_ext(name, ".mkv");
}

// 释放文件列表内存的辅助函数
static void free_file_list(char** list, int count) {
  if (list) {
    for (int i = 0; i < count; ++i) {
      free(list[i]);
    }
    free(list);
  }
}

// ================= 子页：图片  =================
typedef struct {
  lv_obj_t* img;
  const char* path;
} ImgItemCtx;
// ===================== 终极调试函数 =====================
static void on_image_item_clicked(lv_event_t* e) {
  ImgItemCtx* ctx = (ImgItemCtx*)lv_event_get_user_data(e);
  if (ctx && ctx->img && ctx->path) {
    LV_LOG_USER("[images] load -> %s", ctx->path);
    lv_img_set_src(ctx->img, ctx->path);
    lv_obj_center(ctx->img);
  }
}

void screen_entertainment_images(void) {
  lv_obj_t* scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  // 顶栏
  sh_create_topbar("图片 Images");

  // 主容器（卡片）
  lv_obj_t* card = lv_obj_create(scr);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 480 - 48 - 24);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 48 + 12);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_all(card, 12, 0);
  lv_obj_set_style_pad_column(card, 12, 0);
  lv_obj_set_style_pad_row(card, 12, 0);

  // 左：文件列表
  lv_obj_t* left = lv_obj_create(card);
  lv_obj_remove_style_all(left);
  lv_obj_set_size(left, 260, LV_PCT(100));
  lv_obj_set_style_bg_color(left, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(left, 1, 0);
  lv_obj_set_style_border_color(left, lv_palette_lighten(LV_PALETTE_GREY, 2),
                                0);
  lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(left, 8, 0);
  lv_obj_set_style_pad_row(left, 6, 0);
  lv_obj_set_scroll_dir(left, LV_DIR_VER);

  // 右：预览
  lv_obj_t* right = lv_obj_create(card);
  lv_obj_remove_style_all(right);
  lv_obj_set_height(right, LV_PCT(100));
  lv_obj_set_flex_grow(right, 1);
  lv_obj_set_style_bg_color(right, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(right, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(right, 1, 0);
  lv_obj_set_style_border_color(right, lv_palette_lighten(LV_PALETTE_GREY, 2),
                                0);
  lv_obj_set_style_pad_all(right, 8, 0);

  lv_obj_t* img = lv_img_create(right);
  lv_obj_center(img);

  // 读取目录，列出图片（从工作目录下的 ./media/）
  DIR* d = opendir("./media");
  if (d) {
    struct dirent* de;
    while ((de = readdir(d)) != NULL) {
      if (de->d_name[0] == '.') continue;
      if (!is_image_file(de->d_name)) continue;
      LV_LOG_USER("[images] found file: %s", de->d_name);
      lv_obj_t* row = lv_btn_create(left);
      lv_obj_set_size(row, LV_PCT(100), 32);
      lv_obj_add_style(row, sh_style_btn_neutral(),
                       LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_style(row, sh_style_btn_neutral_pressed(),
                       LV_PART_MAIN | LV_STATE_PRESSED);
      lv_obj_set_style_pad_left(row, 8, 0);
      lv_obj_set_style_pad_right(row, 8, 0);
      lv_obj_t* lbl = lv_label_create(row);
      lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
      lv_label_set_text(lbl, de->d_name);
      lv_obj_center(lbl);
      // 为按钮准备事件上下文：包含右侧 img 与路径
      ImgItemCtx* ictx = (ImgItemCtx*)malloc(sizeof(ImgItemCtx));
      if (ictx) {
        ictx->img = img;
        const char* prefix = "A:./media/";
        size_t plen = strlen(prefix) + strlen(de->d_name) + 1;
        char* full = (char*)malloc(plen);
        if (full) {
          snprintf(full, plen, "%s%s", prefix, de->d_name);
        }
        ictx->path = full ? full : strdup(de->d_name);
      }
      lv_obj_add_event_cb(row, on_image_item_clicked, LV_EVENT_CLICKED, ictx);
    }
    closedir(d);
  } else {
    lv_obj_t* lbl = lv_label_create(left);
    lv_obj_add_style(lbl, sh_style_text_zh(), 0);
    lv_label_set_text(lbl, "无法打开目录");
  }

  // 图片页返回按钮
  lv_obj_t* btn_back_img = lv_btn_create(scr);
  lv_obj_set_size(btn_back_img, 50, 50);
  lv_obj_set_style_radius(btn_back_img, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(btn_back_img, LV_ALIGN_BOTTOM_LEFT, 25, -25);
  lv_obj_add_event_cb(btn_back_img, on_back_to_entertainment, LV_EVENT_CLICKED,
                      NULL);
  lv_label_set_text(lv_label_create(btn_back_img), LV_SYMBOL_LEFT);
}

// ================= 子页：音乐 =================

static lv_obj_t* g_music_title_label;
static lv_obj_t* g_music_play_btn_label;

// 播放指定索引的音乐
static void play_music_at_index(int index) {
  if (index < 0 || index >= g_music_count) return;

  // 确保之前的播放已停止
  system("killall -9 madplay");
  sleep(1);  // 等待进程完全终止

  char cmd[512];
  snprintf(cmd, sizeof(cmd), "madplay ./media/%s &", g_music_files[index]);
  system(cmd);

  g_current_music_index = index;
  g_is_music_playing = true;

  // 更新UI
  lv_label_set_text(g_music_title_label, g_music_files[index]);
  lv_label_set_text(g_music_play_btn_label, LV_SYMBOL_PAUSE);
}

// 音乐列表项点击事件
static void on_music_item_clicked(lv_event_t* e) {
  intptr_t index = (intptr_t)lv_event_get_user_data(e);
  play_music_at_index(index);
}

static void on_music_play_pause(lv_event_t* e) {
  LV_UNUSED(e);
  if (g_current_music_index == -1) {  // 如果从未播放过，播放第一首
    play_music_at_index(0);
    return;
  }

  if (g_is_music_playing) {  // 正在播放 -> 暂停
    system("killall -19 madplay");
    lv_label_set_text(g_music_play_btn_label, LV_SYMBOL_PLAY);
    g_is_music_playing = false;
  } else {  // 已暂停 -> 继续
    system("killall -18 madplay");
    lv_label_set_text(g_music_play_btn_label, LV_SYMBOL_PAUSE);
    g_is_music_playing = true;
  }
}

static void on_music_prev(lv_event_t* e) {
  LV_UNUSED(e);
  if (g_music_count == 0) return;
  int prev_index = (g_current_music_index - 1 + g_music_count) % g_music_count;
  play_music_at_index(prev_index);
}

static void on_music_next(lv_event_t* e) {
  LV_UNUSED(e);
  if (g_music_count == 0) return;
  int next_index = (g_current_music_index + 1) % g_music_count;
  play_music_at_index(next_index);
}

static void on_music_stop(lv_event_t* e) {
  LV_UNUSED(e);
  system("killall -9 madplay");
  g_is_music_playing = false;
  g_current_music_index = -1;
  lv_label_set_text(g_music_title_label, "请选择歌曲");
  lv_label_set_text(g_music_play_btn_label, LV_SYMBOL_PLAY);
}

// 音乐页返回处理
static void on_music_back(lv_event_t* e) {
  LV_UNUSED(e);
  on_music_stop(NULL);                           // 退出时停止播放
  free_file_list(g_music_files, g_music_count);  // 释放内存
  g_music_files = NULL;
  g_music_count = 0;
  demo_entertainment();
}

void screen_entertainment_music(void) {
  lv_obj_t* scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("音乐 Music");

  lv_obj_t* card = lv_obj_create(scr);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 480 - 48 - 24);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 48 + 12);
  lv_obj_set_style_pad_all(card, 12, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);  // 改为行布局

  // 左侧：音乐列表
  lv_obj_t* left = lv_obj_create(card);
  lv_obj_remove_style_all(left);
  lv_obj_set_size(left, 260, LV_PCT(100));
  lv_obj_set_style_bg_color(left, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(left, 1, 0);
  lv_obj_set_style_border_color(left, lv_palette_lighten(LV_PALETTE_GREY, 2),
                                0);
  lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(left, 8, 0);
  lv_obj_set_style_pad_row(left, 6, 0);
  lv_obj_set_scroll_dir(left, LV_DIR_VER);

  // 右侧：播放控制
  lv_obj_t* right = lv_obj_create(card);
  lv_obj_remove_style_all(right);
  lv_obj_set_height(right, LV_PCT(100));
  lv_obj_set_flex_grow(right, 1);
  lv_obj_set_style_pad_all(right, 12, 0);
  lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(right, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(right, 5, 0);

  // 歌曲信息
  g_music_title_label = lv_label_create(right);
  lv_obj_add_style(g_music_title_label, sh_style_text_zh_large(), 0);
  lv_label_set_text(g_music_title_label, "请选择歌曲");

  // 进度条
  lv_obj_t* slider = lv_slider_create(right);
  lv_obj_set_width(slider, LV_PCT(100));

  // 控制按钮行
  lv_obj_t* row = lv_obj_create(right);
  lv_obj_remove_style_all(row);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 20, 0);

  lv_obj_t* btn_prev = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_prev), LV_SYMBOL_PREV);
  lv_obj_add_event_cb(btn_prev, on_music_prev, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_play = lv_btn_create(row);
  g_music_play_btn_label = lv_label_create(btn_play);
  lv_label_set_text(g_music_play_btn_label, LV_SYMBOL_PLAY);
  lv_obj_add_event_cb(btn_play, on_music_play_pause, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_next = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_next), LV_SYMBOL_NEXT);
  lv_obj_add_event_cb(btn_next, on_music_next, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_stop = lv_btn_create(row);
  lv_label_set_text(lv_label_create(btn_stop), LV_SYMBOL_STOP);
  lv_obj_add_event_cb(btn_stop, on_music_stop, LV_EVENT_CLICKED, NULL);

  // 扫描音乐文件
  DIR* d = opendir("./media");
  if (d) {
    struct dirent* de;
    while ((de = readdir(d)) != NULL) {
      if (is_music_file(de->d_name)) {
        g_music_count++;
        g_music_files =
            (char**)realloc(g_music_files, g_music_count * sizeof(char*));
        g_music_files[g_music_count - 1] = strdup(de->d_name);
      }
    }
    closedir(d);

    // 创建列表项
    for (int i = 0; i < g_music_count; i++) {
      lv_obj_t* list_btn = lv_btn_create(left);
      lv_obj_set_size(list_btn, LV_PCT(100), 32);
      lv_obj_add_style(list_btn, sh_style_btn_neutral(),
                       LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_style(list_btn, sh_style_btn_neutral_pressed(),
                       LV_PART_MAIN | LV_STATE_PRESSED);
      lv_obj_add_event_cb(list_btn, on_music_item_clicked, LV_EVENT_CLICKED,
                          (void*)(intptr_t)i);

      lv_obj_t* lbl = lv_label_create(list_btn);
      lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
      lv_label_set_text(lbl, g_music_files[i]);
      lv_obj_center(lbl);
    }

  } else {
    lv_obj_t* lbl = lv_label_create(left);
    lv_obj_add_style(lbl, sh_style_text_zh(), 0);
    lv_label_set_text(lbl, "无法打开 ./media 目录");
  }

  // 返回按钮
  lv_obj_t* btn_back = lv_btn_create(scr);
  lv_obj_set_size(btn_back, 50, 50);
  lv_obj_set_style_radius(btn_back, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 25, -25);
  lv_obj_add_event_cb(btn_back, on_music_back, LV_EVENT_CLICKED, NULL);
  lv_label_set_text(lv_label_create(btn_back), LV_SYMBOL_LEFT);
}

// ================= 子页：视频 =================

static lv_obj_t* g_video_title_label;
static lv_obj_t* g_video_play_btn_label;
const char* fifo_path = "/tmp/pipe";

// 向管道发送命令
static void send_mplayer_cmd(const char* cmd) {
  int fd = open(fifo_path, O_WRONLY | O_NONBLOCK);
  if (fd >= 0) {
    write(fd, cmd, strlen(cmd));
    close(fd);
  }
}

// 播放指定索引的视频
static void play_video_at_index(int index) {
  if (index < 0 || index >= g_video_count) return;

  // 如果已有mplayer在运行，先杀掉
  if (g_is_video_playing || g_is_video_paused) {
    system("killall -9 mplayer");
    sleep(1);
  }

  // 启动mplayer
  char cmd[1024];
  snprintf(cmd, sizeof(cmd),
           "mplayer -slave -quiet -noborder -input file=%s -geometry 240:143 "
           "-zoom -x 453 -y 240 ./media/%s &",
           fifo_path, g_video_files[index]);
  system(cmd);

  g_current_video_index = index;
  g_is_video_playing = true;
  g_is_video_paused = false;

  // 更新UI
  lv_label_set_text(g_video_title_label, g_video_files[index]);
  lv_label_set_text(g_video_play_btn_label, LV_SYMBOL_PAUSE);
}

static void on_video_item_clicked(lv_event_t* e) {
  intptr_t index = (intptr_t)lv_event_get_user_data(e);
  play_video_at_index(index);
}

static void on_video_play_pause(lv_event_t* e) {
  LV_UNUSED(e);
  if (g_current_video_index == -1) {
    play_video_at_index(0);
    return;
  }
  send_mplayer_cmd("pause\n");
  g_is_video_paused = !g_is_video_paused;
  lv_label_set_text(g_video_play_btn_label,
                    g_is_video_paused ? LV_SYMBOL_PLAY : LV_SYMBOL_PAUSE);
}

static void on_video_stop(lv_event_t* e) {
  LV_UNUSED(e);
  send_mplayer_cmd("quit\n");
  g_is_video_playing = false;
  g_is_video_paused = false;
  g_current_video_index = -1;
  lv_label_set_text(g_video_title_label, "请选择视频");
  lv_label_set_text(g_video_play_btn_label, LV_SYMBOL_PLAY);
}

// 视频页返回
static void on_video_back(lv_event_t* e) {
  LV_UNUSED(e);
  on_video_stop(NULL);           // 尝试优雅退出
  system("killall -9 mplayer");  // 强制清理，以防万一
  free_file_list(g_video_files, g_video_count);
  g_video_files = NULL;
  g_video_count = 0;
  demo_entertainment();
}

void screen_entertainment_video(void) {
  // 创建管道
  mkfifo(fifo_path, 0777);

  lv_obj_t* scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("视频 Video");

  lv_obj_t* card = lv_obj_create(scr);
  lv_obj_add_style(card, sh_style_card(), 0);
  lv_obj_set_size(card, 800 - 24, 480 - 48 - 24);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 48 + 12);
  lv_obj_set_style_pad_all(card, 12, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(card, 12, 0);  // 为左右面板添加间距

  // 左侧：视频列表
  lv_obj_t* left = lv_obj_create(card);
  lv_obj_remove_style_all(left);
  lv_obj_set_width(left, 183);
  lv_obj_set_height(left, LV_PCT(100));  // 左侧高度占满
  lv_obj_set_style_bg_color(left, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(left, 1, 0);
  lv_obj_set_style_border_color(left, lv_palette_lighten(LV_PALETTE_GREY, 2),
                                0);
  lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(left, 8, 0);
  lv_obj_set_style_pad_row(left, 6, 0);
  lv_obj_set_scroll_dir(left, LV_DIR_VER);

  // 右侧：视频播放区域和控制
  lv_obj_t* right = lv_obj_create(card);
  lv_obj_remove_style_all(right);
  lv_obj_set_flex_grow(right, 1);         // 让 right 自动填充 card 的剩余宽度
  lv_obj_set_height(right, LV_PCT(100));  // 让 right 和 card 一样高
  lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);  // 内部元素垂直排列
  lv_obj_set_style_pad_all(right, 0, 0);             // 移除内边距，方便精确控制
  lv_obj_set_style_pad_row(right, 15, 0);            // 设置元素间的垂直间距

  // 视频标题
  g_video_title_label = lv_label_create(right);
  lv_obj_add_style(g_video_title_label, sh_style_text_zh_large(), 0);
  lv_label_set_text(g_video_title_label, "请选择视频");
  lv_obj_set_width(g_video_title_label, LV_PCT(100));
  lv_label_set_long_mode(g_video_title_label, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_align(g_video_title_label, LV_TEXT_ALIGN_CENTER,
                              0);  // 标题居中

  // 视频显示占位 (mplayer窗口会覆盖在这里)
  lv_obj_t* video_area = lv_obj_create(right);
  lv_obj_set_size(video_area, LV_PCT(100), 240);
  lv_obj_set_style_bg_color(video_area, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(video_area, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(video_area, 8, 0);  // 添加圆角更美观
  lv_obj_clear_flag(video_area, LV_OBJ_FLAG_SCROLLABLE);

  // 控制按钮容器
  lv_obj_t* controls_container = lv_obj_create(right);
  lv_obj_remove_style_all(controls_container);
  lv_obj_set_width(controls_container, LV_PCT(100));
  lv_obj_set_height(controls_container, LV_SIZE_CONTENT);  // 高度由内容决定
  lv_obj_set_flex_flow(controls_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(controls_container, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(controls_container, 20, 0);

  lv_obj_t* btn_play = lv_btn_create(controls_container);
  g_video_play_btn_label = lv_label_create(btn_play);
  lv_label_set_text(g_video_play_btn_label, LV_SYMBOL_PLAY);
  lv_obj_add_event_cb(btn_play, on_video_play_pause, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_stop = lv_btn_create(controls_container);
  lv_label_set_text(lv_label_create(btn_stop), LV_SYMBOL_STOP);
  lv_obj_add_event_cb(btn_stop, on_video_stop, LV_EVENT_CLICKED, NULL);

  DIR* d = opendir("./media");
  if (d) {
    struct dirent* de;
    while ((de = readdir(d)) != NULL) {
      if (is_video_file(de->d_name)) {
        g_video_count++;
        g_video_files =
            (char**)realloc(g_video_files, g_video_count * sizeof(char*));
        g_video_files[g_video_count - 1] = strdup(de->d_name);
      }
    }
    closedir(d);
    for (int i = 0; i < g_video_count; i++) {
      lv_obj_t* list_btn = lv_btn_create(left);
      lv_obj_set_size(list_btn, LV_PCT(100), 32);
      lv_obj_add_style(list_btn, sh_style_btn_neutral(),
                       LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_style(list_btn, sh_style_btn_neutral_pressed(),
                       LV_PART_MAIN | LV_STATE_PRESSED);
      lv_obj_add_event_cb(list_btn, on_video_item_clicked, LV_EVENT_CLICKED,
                          (void*)(intptr_t)i);
      lv_obj_t* lbl = lv_label_create(list_btn);
      lv_obj_add_style(lbl, sh_style_text_zh_small(), 0);
      lv_label_set_text(lbl, g_video_files[i]);
      lv_obj_center(lbl);
    }
  } else {
    lv_obj_t* lbl = lv_label_create(left);
    lv_obj_add_style(lbl, sh_style_text_zh(), 0);
    lv_label_set_text(lbl, "无法打开 ./media 目录");
  }

  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* btn_back = lv_btn_create(scr);
  lv_obj_set_size(btn_back, 50, 50);
  lv_obj_set_style_radius(btn_back, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 25, -25);
  lv_obj_add_event_cb(btn_back, on_video_back, LV_EVENT_CLICKED, NULL);
  lv_label_set_text(lv_label_create(btn_back), LV_SYMBOL_LEFT);
}

// ================= 娱乐主页 =================
static void on_go_images(lv_event_t* e) {
  LV_UNUSED(e);
  screen_entertainment_images();
}
static void on_go_music(lv_event_t* e) {
  LV_UNUSED(e);
  screen_entertainment_music();
}
static void on_go_video(lv_event_t* e) {
  LV_UNUSED(e);
  screen_entertainment_video();
}

void screen_entertainment_build(void) {
  lv_obj_t* scr = lv_scr_act();
  lv_obj_clean(scr);
  sh_init_styles_once();

  sh_create_topbar("娱乐 Entertainment");

  // 内容容器
  lv_obj_t* cont = lv_obj_create(scr);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, 800, 480 - 48);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
  lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(cont, 12, 0);

  // 三张卡片：图片/音乐/视频
  lv_obj_t* grid = lv_obj_create(cont);
  lv_obj_remove_style_all(grid);
  lv_obj_set_size(grid, 800 - 24, 480 - 120);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(grid, 12, 0);
  lv_obj_set_style_pad_row(grid, 12, 0);
  lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);

  const char* names[3] = {"图片", "音乐", "视频"};
  lv_event_cb_t cbs[3] = {on_go_images, on_go_music, on_go_video};

  for (int i = 0; i < 3; ++i) {
    lv_obj_t* card = lv_obj_create(grid);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, (800 - 24 - 12 * 2) / 3, 320);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* btn = lv_btn_create(card);
    lv_obj_add_style(btn, sh_style_btn_neutral(),
                     LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(btn, sh_style_btn_neutral_pressed(),
                     LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_size(btn, LV_PCT(100), LV_PCT(100));
    lv_obj_add_event_cb(btn, cbs[i], LV_EVENT_CLICKED, NULL);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_obj_add_style(lbl, sh_style_text_zh_large(), 0);
    lv_label_set_text(lbl, names[i]);
    lv_obj_center(lbl);
  }
}