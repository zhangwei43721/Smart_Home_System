#include "obj/Include/screens_common.h"
#include "obj/Include/screen_ac.h"

// 外部导航（在 screens_home.c 或其他地方定义）
extern void demo_dashboard(void); // 假设这是返回主页的函数

// --- 全局静态数据模型 ---
// 初始化空调的默认状态
static ACState g_ac_state = {
    .is_on = true,
    .temperature_target = 24,
    .temperature_current = 26, // 模拟一个当前室温
    .mode = AC_MODE_COOL,
    .fan_speed = AC_FAN_AUTO,
    .swing_on = false,
};


// --- 前向声明 ---
static void update_ac_ui(void);


/* ===============================
 *  事件回调函数
 * ===============================*/

// 返回按钮事件
static void on_back_event(lv_event_t *e) {
    LV_UNUSED(e);
    demo_dashboard();
}

// 电源开关事件
static void on_power_toggle(lv_event_t *e) {
    g_ac_state.is_on = !g_ac_state.is_on; // 只修改数据
    update_ac_ui(); // 调用UI刷新
}

// 温度调节事件 (+/-)
static void on_temp_change(lv_event_t *e) {
    intptr_t delta = (intptr_t)lv_event_get_user_data(e);
    int new_temp = g_ac_state.temperature_target + delta;

    if (new_temp >= 16 && new_temp <= 30) { // 温度范围限制
        g_ac_state.temperature_target = new_temp;
        update_ac_ui();
    }
}

// 模式选择事件
static void on_mode_select(lv_event_t *e) {
    ACMode new_mode = (ACMode)(intptr_t)lv_event_get_user_data(e);
    g_ac_state.mode = new_mode;
    update_ac_ui();
}

// 风速选择事件
static void on_fan_select(lv_event_t *e) {
    ACFanSpeed new_speed = (ACFanSpeed)(intptr_t)lv_event_get_user_data(e);
    g_ac_state.fan_speed = new_speed;
    update_ac_ui();
}

// 摆风开关事件
static void on_swing_toggle(lv_event_t *e) {
    g_ac_state.swing_on = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    update_ac_ui();
}


/* ===============================
 *  核心UI更新函数
 * ===============================*/

// 数据驱动UI的核心：根据 g_ac_state 的当前数据，刷新整个UI界面
static void update_ac_ui(void) {
    ACState *state = &g_ac_state;

    // 1. 更新温度显示
    lv_label_set_text_fmt(state->ui.label_temp_target, "%d", state->temperature_target);

    // 2. 更新电源按钮和所有其他控件的可用状态
    if (state->is_on) {
        lv_obj_add_state(state->ui.btn_power, LV_STATE_CHECKED);
        // 移除禁用状态，让控件可操作
        lv_obj_clear_state(state->ui.controls_panel, LV_STATE_DISABLED);
        lv_obj_set_style_text_color(state->ui.label_temp_target, lv_color_hex(0x007bff), 0);
    } else {
        lv_obj_clear_state(state->ui.btn_power, LV_STATE_CHECKED);
        // 添加禁用状态，控件变灰且不可点击
        lv_obj_add_state(state->ui.controls_panel, LV_STATE_DISABLED);
        lv_obj_set_style_text_color(state->ui.label_temp_target, lv_palette_main(LV_PALETTE_GREY), 0);
    }

    // 3. 更新模式按钮的选中状态
    for (int i = 0; i < 4; i++) {
        if (state->mode == i) {
            lv_obj_add_state(state->ui.mode_btns[i], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(state->ui.mode_btns[i], LV_STATE_CHECKED);
        }
    }

    // 4. 更新风速按钮的选中状态
    for (int i = 0; i < 4; i++) {
        if (state->fan_speed == i) {
            lv_obj_add_state(state->ui.fan_btns[i], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(state->ui.fan_btns[i], LV_STATE_CHECKED);
        }
    }
    
    // 5. 更新摆风开关状态
    if (state->swing_on) {
        lv_obj_add_state(state->ui.swing_switch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(state->ui.swing_switch, LV_STATE_CHECKED);
    }
}


/* ===============================
 *  UI构建函数
 * ===============================*/

// 辅助函数：创建一个带标签的按钮组容器
static lv_obj_t* create_control_group(lv_obj_t* parent, const char* title) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_width(cont, LV_PCT(100));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(cont, 5, 0);

    lv_obj_t* label = lv_label_create(cont);
    lv_label_set_text(label, title);
    lv_obj_add_style(label, sh_style_text_zh_small(), 0);
    lv_obj_set_style_text_color(label, lv_palette_darken(LV_PALETTE_GREY, 2), 0);

    lv_obj_t* btn_row = lv_obj_create(cont);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_width(btn_row, LV_PCT(100));
    lv_obj_set_layout(btn_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(btn_row, 5, 0);

    return btn_row;
}


void screen_ac_build(void) {
    // 1. 清屏并创建基础布局（顶栏和背景容器）
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    sh_init_styles_once();

    lv_obj_t *top = sh_create_topbar("空调 Air Conditioner");
    LV_UNUSED(top);

    lv_obj_t * cont = lv_obj_create(scr);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 800, 480 - 48);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_bg_color(cont, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
    lv_obj_set_style_pad_all(cont, 12, 0);

    // 2. 创建主控制卡片
    lv_obj_t *card = lv_obj_create(cont);
    lv_obj_add_style(card, sh_style_card(), 0);
    lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW); // 左右两栏布局
    lv_obj_set_style_pad_all(card, 20, 0);

    // --- 左侧：温度显示与调节 ---
    lv_obj_t* left_panel = lv_obj_create(card);
    lv_obj_remove_style_all(left_panel);
    lv_obj_set_size(left_panel, 350, LV_PCT(100));
    lv_obj_set_flex_flow(left_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(left_panel, 15, 0);

    lv_obj_t* lbl_curr = lv_label_create(left_panel);
    lv_obj_add_style(lbl_curr, sh_style_text_zh(), 0);
    lv_label_set_text_fmt(lbl_curr, "当前室温 %d°C", g_ac_state.temperature_current);
    
    g_ac_state.ui.label_temp_target = lv_label_create(left_panel);
    // 这里为大号数字温度应用中文大号样式
    lv_obj_add_style(g_ac_state.ui.label_temp_target, sh_style_text_zh_large(), 0);
    // 如果未启用 48 号 Montserrat 字体，则回退到中文常用字体或默认字体
#if LV_FONT_MONTSERRAT_48
    lv_obj_set_style_text_font(g_ac_state.ui.label_temp_target, &lv_font_montserrat_48, 0);
#else
    if (sh_get_font_zh()) lv_obj_set_style_text_font(g_ac_state.ui.label_temp_target, sh_get_font_zh(), 0);
#endif

    lv_obj_t* btn_row = lv_obj_create(left_panel);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_width(btn_row, LV_PCT(100));
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_minus = lv_btn_create(btn_row);
    lv_obj_set_size(btn_minus, 60, 60);
    lv_obj_add_event_cb(btn_minus, on_temp_change, LV_EVENT_CLICKED, (void*)-1);
    {
        lv_obj_t* lbl = lv_label_create(btn_minus);
        lv_obj_add_style(lbl, sh_style_text_zh(), 0);
        lv_label_set_text(lbl, "-");
    }

    lv_obj_t* btn_plus = lv_btn_create(btn_row);
    lv_obj_set_size(btn_plus, 60, 60);
    lv_obj_add_event_cb(btn_plus, on_temp_change, LV_EVENT_CLICKED, (void*)1);
    {
        lv_obj_t* lbl = lv_label_create(btn_plus);
        lv_obj_add_style(lbl, sh_style_text_zh(), 0);
        lv_label_set_text(lbl, "+");
    }
    
    // --- 右侧：功能开关与选项 ---
    lv_obj_t* right_panel = lv_obj_create(card);
    g_ac_state.ui.controls_panel = right_panel; // 保存这个面板的指针，用于整体禁用
    lv_obj_remove_style_all(right_panel);
    lv_obj_set_flex_grow(right_panel, 1); // 占据剩余空间
    lv_obj_set_height(right_panel, LV_PCT(100));
    lv_obj_set_flex_flow(right_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_panel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 电源与摆风
    lv_obj_t* top_row = lv_obj_create(right_panel);
    lv_obj_remove_style_all(top_row);
    lv_obj_set_width(top_row, LV_PCT(100));
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_ac_state.ui.btn_power = lv_btn_create(top_row);
    lv_obj_add_flag(g_ac_state.ui.btn_power, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(g_ac_state.ui.btn_power, 80, 40);
    lv_obj_add_event_cb(g_ac_state.ui.btn_power, on_power_toggle, LV_EVENT_CLICKED, NULL);
    {
        lv_obj_t* lbl = lv_label_create(g_ac_state.ui.btn_power);
        lv_obj_add_style(lbl, sh_style_text_zh(), 0);
        lv_label_set_text(lbl, "电源");
    }
    
    g_ac_state.ui.swing_switch = lv_switch_create(top_row);
    lv_obj_add_event_cb(g_ac_state.ui.swing_switch, on_swing_toggle, LV_EVENT_VALUE_CHANGED, NULL);

    // 模式
    const char* mode_texts[] = {"制冷", "制热", "送风", "除湿"};
    lv_obj_t* mode_row = create_control_group(right_panel, "模式");
    for (int i = 0; i < 4; i++) {
        g_ac_state.ui.mode_btns[i] = lv_btn_create(mode_row);
        lv_obj_add_flag(g_ac_state.ui.mode_btns[i], LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(g_ac_state.ui.mode_btns[i], 1); // 均分宽度
        lv_obj_add_event_cb(g_ac_state.ui.mode_btns[i], on_mode_select, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        {
            lv_obj_t* lbl = lv_label_create(g_ac_state.ui.mode_btns[i]);
            lv_obj_add_style(lbl, sh_style_text_zh(), 0);
            lv_label_set_text(lbl, mode_texts[i]);
        }
    }

    // 风速
    const char* fan_texts[] = {"自动", "低", "中", "高"};
    lv_obj_t* fan_row = create_control_group(right_panel, "风速");
    for (int i = 0; i < 4; i++) {
        g_ac_state.ui.fan_btns[i] = lv_btn_create(fan_row);
        lv_obj_add_flag(g_ac_state.ui.fan_btns[i], LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(g_ac_state.ui.fan_btns[i], 1); // 均分宽度
        lv_obj_add_event_cb(g_ac_state.ui.fan_btns[i], on_fan_select, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        {
            lv_obj_t* lbl = lv_label_create(g_ac_state.ui.fan_btns[i]);
            lv_obj_add_style(lbl, sh_style_text_zh(), 0);
            lv_label_set_text(lbl, fan_texts[i]);
        }
    }
    
    // 3. 创建左上角返回按钮
    lv_obj_t * btn_back = lv_btn_create(scr);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_set_style_radius(btn_back, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_add_event_cb(btn_back, on_back_event, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(btn_back), LV_SYMBOL_LEFT);
    
    // 4. 最后，调用一次 update_ac_ui() 来根据初始数据设置UI的正确状态
    update_ac_ui();
}