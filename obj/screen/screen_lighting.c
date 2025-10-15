#include "obj/Include/screen_lighting.h"
#include "obj/Include/screens_common.h"

// --- 全局静态数据 ---
// 使用新的数据结构初始化所有灯光设备，作为本页面的“数据模型”
#define LIGHT_COUNT 8
static LightDevice g_lights[LIGHT_COUNT] = {
    {.name = "客厅主灯",   .is_on = true,  .supports_adjust = true, .brightness = 80, .color_temp = 4000},
    {.name = "餐厅吊灯",   .is_on = false, .supports_adjust = true, .brightness = 70, .color_temp = 3500},
    {.name = "卧室筒灯",   .is_on = true,  .supports_adjust = true, .brightness = 60, .color_temp = 3000},
    {.name = "走廊灯",     .is_on = false, .supports_adjust = false}, // 不支持调节的灯
    {.name = "书房台灯",   .is_on = false, .supports_adjust = true, .brightness = 90, .color_temp = 4500},
    {.name = "阳台灯",     .is_on = false, .supports_adjust = false},
    {.name = "卫生间灯",   .is_on = false, .supports_adjust = false},
    {.name = "厨房灯",     .is_on = false, .supports_adjust = false},
};

// 用于向调节弹窗传递数据的上下文结构体
typedef struct {
    int light_index;
    lv_obj_t* mask; // 指向蒙层，方便关闭
} DialogContext;


// --- 前向声明静态函数 ---
static void update_card_ui(LightDevice* light);


/* ===============================
 *  调节弹窗相关的事件处理
 * ===============================*/

// 弹窗上的亮度滑块值改变时调用
static void on_dialog_bri_changed(lv_event_t *e) {
    DialogContext *ctx = (DialogContext *)lv_event_get_user_data(e);
    lv_obj_t *slider = lv_event_get_target(e);
    g_lights[ctx->light_index].brightness = lv_slider_get_value(slider);
    update_card_ui(&g_lights[ctx->light_index]); // 实时更新卡片上的状态文字
}

// 弹窗上的色温滑块值改变时调用
static void on_dialog_ct_changed(lv_event_t *e) {
    DialogContext *ctx = (DialogContext *)lv_event_get_user_data(e);
    lv_obj_t *slider = lv_event_get_target(e);
    g_lights[ctx->light_index].color_temp = lv_slider_get_value(slider);
    update_card_ui(&g_lights[ctx->light_index]);
}

// 关闭弹窗的事件 (给蒙层和完成按钮使用)
static void on_dialog_close(lv_event_t *e) {
    DialogContext *ctx = (DialogContext *)lv_event_get_user_data(e);
    if(ctx && ctx->mask) {
        lv_obj_del(ctx->mask); // 直接删除蒙层，其所有子对象(弹窗)也会被一并删除
        lv_mem_free(ctx);      // 释放我们为上下文分配的内存，防止内存泄漏
    }
}


/* ===============================
 *  灯光卡片相关的事件处理
 * ===============================*/

// 点击卡片本身，打开调节弹窗
static void on_card_click(lv_event_t *e) {
    intptr_t idx = (intptr_t)lv_event_get_user_data(e);

    // 如果灯不支持调节，则不响应点击
    if (!g_lights[idx].supports_adjust) {
        return;
    }

    // 动态分配上下文内存，确保多个弹窗同时存在时（虽然不太可能）不会冲突
    DialogContext* ctx = lv_mem_alloc(sizeof(DialogContext));
    ctx->light_index = idx;

    // 1. 创建一个半透明的蒙层，覆盖整个屏幕
    lv_obj_t* mask = lv_obj_create(lv_scr_act());
    ctx->mask = mask; // 在上下文中保存蒙层指针
    lv_obj_remove_style_all(mask);
    lv_obj_set_size(mask, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(mask, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(mask, LV_OPA_50, 0);
    // 点击蒙层本身即可关闭弹窗
    lv_obj_add_event_cb(mask, on_dialog_close, LV_EVENT_CLICKED, ctx);

    // 2. 创建弹窗面板，作为蒙层的子对象
    lv_obj_t * dlg = lv_obj_create(mask);
    lv_obj_remove_style_all(dlg);
    lv_obj_set_size(dlg, 360, 240); // 增加一点高度以容纳标题
    lv_obj_center(dlg);
    lv_obj_set_style_bg_color(dlg, lv_color_white(), 0);
    lv_obj_set_style_radius(dlg, 12, 0);
    lv_obj_set_style_pad_all(dlg, 15, 0);
    lv_obj_set_flex_flow(dlg, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(dlg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(dlg, 10, 0); // 设置行间距

    // 弹窗标题
    lv_obj_t * lbl_title = lv_label_create(dlg);
    lv_label_set_text_fmt(lbl_title, "%s 调节", g_lights[idx].name);
    lv_obj_add_style(lbl_title, sh_style_text_zh(), 0);

    // 亮度
    lv_obj_t * lbl_b = lv_label_create(dlg);
    lv_label_set_text(lbl_b, "亮度");
    lv_obj_add_style(lbl_b, sh_style_text_zh(), 0);
    lv_obj_t * s_b = lv_slider_create(dlg);
    lv_obj_set_width(s_b, LV_PCT(100));
    lv_slider_set_range(s_b, 0, 100);
    lv_slider_set_value(s_b, g_lights[idx].brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(s_b, on_dialog_bri_changed, LV_EVENT_VALUE_CHANGED, ctx);

    // 色温
    lv_obj_t * lbl_c = lv_label_create(dlg);
    lv_label_set_text(lbl_c, "色温");
  lv_obj_add_style(lbl_c, sh_style_text_zh(), 0);
    lv_obj_t * s_c = lv_slider_create(dlg);
    lv_obj_set_width(s_c, LV_PCT(100));
    lv_slider_set_range(s_c, 2700, 6500);
    lv_slider_set_value(s_c, g_lights[idx].color_temp, LV_ANIM_OFF);
    lv_obj_add_event_cb(s_c, on_dialog_ct_changed, LV_EVENT_VALUE_CHANGED, ctx);
    
    // “完成” 按钮
    lv_obj_t * ok = lv_btn_create(mask); // 将按钮也创建在蒙层上
    lv_obj_set_size(ok, 88, 36);
    lv_obj_align_to(ok, dlg, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(ok, on_dialog_close, LV_EVENT_CLICKED, ctx);
    lv_obj_t* lb = lv_label_create(ok);
    lv_label_set_text(lb, "完成");
    lv_obj_add_style(lb, sh_style_text_zh(), 0);
    lv_obj_center(lb);
}

// 点击卡片上的开关时调用
static void on_switch_toggle(lv_event_t *e) {
    intptr_t idx = (intptr_t)lv_event_get_user_data(e);
    lv_obj_t *sw = lv_event_get_target(e);
    
    // 阻止事件冒泡到父对象(卡片)，否则每次点开关都会弹出调节窗口
    lv_event_stop_bubbling(e);

    // 核心原则：事件回调只修改数据模型
    g_lights[idx].is_on = lv_obj_has_state(sw, LV_STATE_CHECKED);
    
    // 让UI更新函数去根据新的数据处理界面变化
    update_card_ui(&g_lights[idx]);
}


/* ===============================
 *  核心UI构建与更新函数
 * ===============================*/

// 根据数据，更新单个卡片的显示状态。这是数据驱动UI的核心。
static void update_card_ui(LightDevice* light) {
    if (!light || !light->card) return;

    lv_obj_t* card = light->card;
    // 使用 lv_obj_get_child 按索引获取子对象可能不稳定，最好用find
    lv_obj_t* name_label = lv_obj_get_child(card, 0); 
    lv_obj_t* status_label = lv_obj_get_child(card, 1);
    lv_obj_t* sw = lv_obj_get_child(card, 2);

    if (light->is_on) {
        // 如果开关状态与数据不符，则更新
        if(!lv_obj_has_state(sw, LV_STATE_CHECKED)) {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        }
        lv_obj_set_style_text_color(name_label, lv_color_black(), 0);

        if (light->supports_adjust) {
            lv_label_set_text_fmt(status_label, "%d%% / %dK", light->brightness, light->color_temp);
        } else {
            lv_label_set_text(status_label, "已开启");
        }
    } else {
        if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
            lv_obj_clear_state(sw, LV_STATE_CHECKED);
        }
        lv_obj_set_style_text_color(name_label, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
        lv_label_set_text(status_label, "已关闭");
    }
}

// 构建整个照明页面的函数
void screen_lighting_build(void) {
    sh_init_styles_once();

    lv_obj_t * scr = lv_scr_act();
    lv_obj_t * cont = lv_obj_create(scr);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 800, 480 - 48 - 56);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_pad_all(cont, 12, 0);
    lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);

    // 栅格容器：用于承载所有卡片
    lv_obj_t * grid = lv_obj_create(cont);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 800 - 24, 480 - 48 - 56 - 24);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_scroll_dir(grid, LV_DIR_VER);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(grid, 12, 0);
    lv_obj_set_style_pad_column(grid, 12, 0);

    // 3列布局尺寸（留白与间距考虑）
    int card_w = (800 - 24 - 12 * 2) / 3;  // 3列
    int card_h = 90;

    for (int i = 0; i < LIGHT_COUNT; ++i) {
        lv_obj_t * card = lv_obj_create(grid);
        lv_obj_remove_style_all(card);
        // lv_obj_add_style(card, sh_style_card(), 0); // 使用你的卡片样式
        lv_obj_set_style_bg_color(card, lv_color_white(), 0);
        lv_obj_set_style_radius(card, 10, 0);
        lv_obj_set_size(card, card_w, card_h);

        g_lights[i].card = card; // 将UI对象与数据关联起来

        // --- 优化的卡片布局和交互 ---
        // 1. 让卡片可点击，用于打开弹窗
        lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(card, on_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        
        // 2. 名称
        lv_obj_t * name = lv_label_create(card);
        lv_label_set_text(name, g_lights[i].name);
        lv_obj_add_style(name, sh_style_text_zh(), 0);
        lv_obj_align(name, LV_ALIGN_TOP_LEFT, 8, 8);
        // lv_obj_set_style_text_font(name, sh_get_font_zh(), 0);

        // 3. 状态标签 (用于显示亮度/色温或开关状态)
        lv_obj_t * status = lv_label_create(card);
        lv_obj_add_style(status, sh_style_text_zh(), 0);
        lv_obj_set_style_text_color(status, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
        lv_obj_align_to(status, name, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

        // 4. 开关
        lv_obj_t * sw = lv_switch_create(card);
        lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -8, 0);
        lv_obj_add_event_cb(sw, on_switch_toggle, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)i);

        // 5. 初始化UI显示
        update_card_ui(&g_lights[i]);
    }
}