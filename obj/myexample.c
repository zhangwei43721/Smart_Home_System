#include "obj/Include/head.h"

/*
===================================================================================
                                笔记配套演示函数
===================================================================================
  说明:
  - 下面的每个 `demo_` 函数都为了演示笔记中的一个特定知识点而设计。
  - 它们都是独立、可运行的。您可以在 main 函数中选择调用其中任何一个。
  -
为了更好的视觉效果，某些演示在笔记代码的基础上增加了一些基础设置（如大小、颜色、文本等）。
-----------------------------------------------------------------------------------
*/

/**
 * @brief 演示1: 创建基础对象 (对应笔记 "代码示例：创建对象")
 * 效果: 在屏幕中央显示一个灰色的面板，面板左上角有一个空的标签。
 */
void demo_create_basic_objects(void) {
  // 获取当前屏幕作为父对象
  lv_obj_t* parent_screen = lv_scr_act();

  // 在屏幕上创建一个面板 (panel) 对象
  lv_obj_t* panel = lv_obj_create(parent_screen);
  // 为了能看见效果，我们给面板设置一个大小并居中
  lv_obj_set_size(panel, 200, 150);
  lv_obj_center(panel);

  // 在面板 (panel) 上创建一个标签 (label) 对象
  lv_obj_t* label = lv_label_create(panel);
  // 为了能看见效果，我们给标签设置一点文字
  lv_label_set_text(label, "A Label");
}

/**
 * @brief 演示2: 设置对象的位置和尺寸
 * 效果: 在屏幕中央显示一个面板，面板顶部中央有一个标签。
 */
void demo_set_position_and_size(void) {
  lv_obj_t* parent_screen = lv_scr_act();
  lv_obj_t* panel = lv_obj_create(parent_screen);

  // 设置面板大小为 200x150 像素
  lv_obj_set_size(panel, 200, 150);
  // 将面板在父对象（此处为屏幕）中居中
  lv_obj_center(panel);

  lv_obj_t* label = lv_label_create(panel);
  lv_label_set_text(label, "Hello, World!");

  // 将标签在面板顶部居中对齐，并向下偏移20像素
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
}

/**
 * @brief 演示3: 创建并应用样式 (对应笔记 "使用样式三部曲")
 * 效果: 在屏幕中央显示一个绿色的、带圆角的按钮。
 */
void demo_create_and_apply_style(void) {
  lv_obj_t* screen = lv_scr_act();

  // 1. 初始化样式
  // 样式变量必须是 static 或全局的，以防函数退出后被销毁
  static lv_style_t style_btn_green;
  lv_style_init(&style_btn_green);

  // 2. 设置样式属性
  // 设置背景颜色为绿色
  lv_style_set_bg_color(&style_btn_green, lv_palette_main(LV_PALETTE_GREEN));
  // 设置圆角半径为 10
  lv_style_set_radius(&style_btn_green, 10);

  // 创建一个按钮对象
  lv_obj_t* btn = lv_btn_create(screen);
  lv_obj_center(btn);  // 居中显示

  // 3. 将样式添加到对象
  lv_obj_add_style(btn, &style_btn_green, LV_STATE_DEFAULT);

  // 给按钮添加一个文本，方便观察
  lv_obj_t* label = lv_label_create(btn);
  lv_label_set_text(label, "Styled Button");
  lv_obj_center(label);
}

// --- 事件处理所需的回调函数 ---
static void my_event_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);             // 获取事件代码
  lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);  // 获取用户数据

  if (code == LV_EVENT_CLICKED) {  // 判断是点击事件
    lv_label_set_text(label, "Button Clicked!");
  }
}

/**
 * @brief 演示4: 添加事件处理 (对应笔记 "事件处理流程")
 * 效果:
 * 屏幕中央有一个面板，面板上有一个标签和按钮。点击按钮后，标签文本会改变。
 */
void demo_add_event_handler(void) {
  lv_obj_t* panel = lv_obj_create(lv_scr_act());
  lv_obj_set_size(panel, 200, 150);
  lv_obj_center(panel);

  // 创建一个标签用于显示结果
  lv_obj_t* info_label = lv_label_create(panel);
  lv_label_set_text(info_label, "Please click the button");
  lv_obj_align(info_label, LV_ALIGN_TOP_MID, 0, 10);

  // 创建一个按钮
  lv_obj_t* btn = lv_btn_create(panel);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 20);

  lv_obj_t* btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "Click Me");
  lv_obj_center(btn_label);

  // 关键步骤：为按钮添加事件回调
  // 将 info_label 作为用户数据传递给回调函数
  lv_obj_add_event_cb(btn, my_event_handler, LV_EVENT_CLICKED, info_label);
}

/**
 * @brief 事件冒泡的回调函数
 * 这个回调函数被父对象和子对象共用，通过判断当前事件目标来识别冒泡。
 */
static void bubbling_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* label_to_update = (lv_obj_t*)lv_event_get_user_data(e);

  // 获取当前正在处理事件的对象
  lv_obj_t* current_target = lv_event_get_current_target(e);
  // 获取最初触发事件的对象
  lv_obj_t* original_target = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (lv_obj_check_type(current_target, &lv_btn_class)) {
      printf("事件由按钮触发 (Child).\n");
    } else if (lv_obj_check_type(current_target, &lv_obj_class)) {
      if (current_target != original_target) {
        printf("事件冒泡到面板 (Parent).\n");
        lv_label_set_text(label_to_update, "Event bubbled to: Panel (Parent)");
      }
    }
  }
}

/**
 * @brief 演示5: 事件冒泡 (Event Bubbling)
 * 效果: 屏幕中央有一个灰色面板(父)，内部有一个按钮(子)。
 *       点击按钮后，事件会先由按钮处理，然后冒泡给父面板再次处理。
 *       上方的标签会依次显示处理事件的对象。
 */
void demo_event_bubbling(void) {
  // 1. 创建父对象 (面板)
  lv_obj_t* parent_panel = lv_obj_create(lv_scr_act());
  lv_obj_set_size(parent_panel, 250, 150);
  lv_obj_center(parent_panel);
  // 父对象必须是可点击的，才能接收到冒泡的点击事件
  lv_obj_add_flag(parent_panel, LV_OBJ_FLAG_CLICKABLE);

  // 创建一个标签用于显示事件处理流程
  lv_obj_t* info_label = lv_label_create(lv_scr_act());
  lv_label_set_text(info_label, "Click the button below");
  lv_obj_align(info_label, LV_ALIGN_TOP_MID, 0, 150);

  // 2. 创建子对象 (按钮)
  lv_obj_t* child_btn = lv_btn_create(parent_panel);
  lv_obj_set_size(child_btn, 120, 50);
  lv_obj_align(child_btn, LV_ALIGN_CENTER, 0, 20);

  lv_obj_t* btn_label = lv_label_create(child_btn);
  lv_label_set_text(btn_label, "Click Me!");
  lv_obj_center(btn_label);

  // 3. (关键步骤) 为子对象开启事件冒泡标志
  lv_obj_add_flag(child_btn, LV_OBJ_FLAG_EVENT_BUBBLE);

  // 4. 为父对象和子对象都绑定同一个事件回调
  //    这样我们才能在同一个函数里观察到事件的传递过程
  lv_obj_add_event_cb(child_btn, bubbling_event_cb, LV_EVENT_CLICKED,
                      info_label);
  lv_obj_add_event_cb(parent_panel, bubbling_event_cb, LV_EVENT_CLICKED,
                      info_label);
}
/*
-----------------------------------------------------------------------------------
                                其他完整功能演示
-----------------------------------------------------------------------------------
*/

/**
 * @brief 演示5: 从内存(数组)加载SJPG图片
 */
// void demo_sjpg_from_array(void) {
//   LV_IMG_DECLARE(abc);  // 声明图片数据所在的数组名称
//   // 这个数组是通过 `https://lvgl.io/tools/imageconverter`
//   // 工具从图片文件转换而来的 放在了此项目的根目录下的 `abc.c` 文件中
//   lv_obj_t* img = lv_img_create(lv_scr_act());
//   lv_img_set_src(img, &abc);
//   lv_obj_center(img);
// }

/**
 * @brief 演示6: 从文件系统加载SJPG图片
 */
void demo_sjpg_from_file(void) {
  lv_obj_t* img = lv_img_create(lv_scr_act());
  // 注意: 路径 "A:/..." 是LVGL文件系统的虚拟路径格式，请确保您的文件系统已挂载
  lv_img_set_src(img, "A:/mnt/nfs/small_image.sjpg");
  lv_obj_center(img);
}

/**
 * @brief 演示7: 使用FreeType加载外部字体并显示文本
 */
void demo_freetype_text(void) {
  // 1. 初始化字体信息
  static lv_ft_info_t info;
  // 确保字体文件在相对编译出的程序路径下
  info.name = "./MiSans.ttf";
  info.weight = 24;
  info.style = FT_FONT_STYLE_NORMAL;
  info.mem = NULL;

  if (!lv_ft_font_init(&info)) {  // 初始化字体
    LV_LOG_ERROR("FreeType 初始化失败.");
    return;
  }

  // 2. 创建一个样式来使用这个字体
  static lv_style_t style;
  lv_style_init(&style);
  // 将刚刚初始化好的字体设置到样式中
  lv_style_set_text_font(&style, info.font);
  lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

  // 3. 创建标签并应用样式
  lv_obj_t* label = lv_label_create(lv_scr_act());
  lv_obj_add_style(label, &style, 0);  // 应用前面设置的style样式
  // 以后显示中文时像上面一样引用这个样式即可
  lv_label_set_text(label, "你好, LVGL\nI'm a font created with FreeType");
  lv_obj_center(label);
}

/**
 * @brief 演示8: 使用Flexbox进行弹性布局
 */
void demo_flex_layout(void) {
  // (代码与您提供的 lv_flex_test 相同，仅重命名)
  // 此处省略字体加载代码，假设已加载或使用默认字体

  /*Create a container with ROW flex direction*/
  lv_obj_t* cont_row = lv_obj_create(lv_scr_act());
  lv_obj_set_size(cont_row, 300, 75);
  lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
  // 给容器加上边框方便观察
  lv_obj_set_style_border_width(cont_row, 1, 0);
  lv_obj_set_style_border_color(cont_row, lv_palette_main(LV_PALETTE_BLUE), 0);

  /*Create a container with COLUMN flex direction*/
  lv_obj_t* cont_col = lv_obj_create(lv_scr_act());
  lv_obj_set_size(cont_col, 200, 150);
  lv_obj_align_to(cont_col, cont_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
  // 给容器加上边框方便观察
  lv_obj_set_style_border_width(cont_col, 1, 0);
  lv_obj_set_style_border_color(cont_col, lv_palette_main(LV_PALETTE_RED), 0);

  for (uint32_t i = 0; i < 4; i++) {
    lv_obj_t* obj;
    lv_obj_t* label;

    /*Add items to the row container*/
    obj = lv_btn_create(cont_row);
    lv_obj_set_flex_grow(obj, 1);  // 允许项目在行内增长

    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "Row %u", i);
    lv_obj_center(label);

    /*Add items to the column container*/
    obj = lv_btn_create(cont_col);
    lv_obj_set_width(obj, lv_pct(100));  // 项目宽度占满容器

    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "Col %u", i);
    lv_obj_center(label);
  }
}