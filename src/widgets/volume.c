#include "volume.h"
#include <lvgl.h>
#include <zephyr/kernel.h>

/* ============================
 *      初始化 Volume Widget
 * ============================ */
int zmk_widget_volume_init(struct zmk_widget_volume *widget, lv_obj_t *parent, struct zmk_widget_clock *clock_widget) {
    widget->volume = 100; // 默认满音量

    // 容器对象
    widget->obj = lv_obj_create(parent);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // 音量条
    widget->bar = lv_bar_create(widget->obj);
    lv_bar_set_range(widget->bar, 0, 100);
    lv_bar_set_value(widget->bar, widget->volume, LV_ANIM_OFF);

    // 设置初始颜色（红色满条）
    lv_obj_set_style_bg_color(widget->bar, lv_color_make(255, 0, 0), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(widget->bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(widget->bar, lv_color_make(80, 80, 80), LV_PART_MAIN); // 背景灰色
    lv_obj_set_style_bg_opa(widget->bar, LV_OPA_COVER, LV_PART_MAIN);

    // ------------------------------
    // 自动计算宽度：小时+分钟+秒
    lv_coord_t w_hm = lv_obj_get_width(clock_widget->label_hm);   // 小时+分钟
    lv_coord_t w_sec = lv_obj_get_width(clock_widget->label_sec); // 秒
    lv_coord_t spacing = 5; // 小时分钟和秒之间的间距
    lv_coord_t total_width = w_hm + w_sec + spacing;

    lv_obj_set_width(widget->bar, total_width);
    lv_obj_set_height(widget->bar, 4);

    // 左对齐，紧贴时钟下方
    lv_obj_align_to(widget->obj, clock_widget->label_hm, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    return 0;
}

/* ============================
 *      获取对象
 * ============================ */
lv_obj_t *zmk_widget_volume_obj(struct zmk_widget_volume *widget) {
    return widget->obj;
}

/* ============================
 *      设置音量（带动画）
 * ============================ */
void zmk_widget_volume_set(struct zmk_widget_volume *widget, int value) {
    if (!widget) return;

    // 限制范围
    if (value < 0) value = 0;
    if (value > 100) value = 100;

    // 配置动画参数
    static lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, widget->bar);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_bar_set_value);
    lv_anim_set_values(&anim, widget->volume, value);
    lv_anim_set_time(&anim, 300);  // 动画时长300ms
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);  // 缓出效果
    lv_anim_start(&anim);

    // 更新当前音量值
    widget->volume = value;

    // -----------------------
    // indicator 颜色分段渐变
    // -----------------------
    lv_color_t color;

    if (value <= 25) {
        color = lv_color_make(0, 255, 0); // 绿
    } else if (value <= 30) {
        // 25~30 渐变 绿→黄
        uint8_t t = (value - 25) * 255 / 5;
        color = lv_color_make(t, 255, 0);
    } else if (value <= 65) {
        color = lv_color_make(255, 255, 0); // 黄
    } else if (value <= 70) {
        // 65~70 渐变 黄→红
        uint8_t t = (value - 65) * 255 / 5;
        color = lv_color_make(255, 255 - t, 0);
    } else {
        color = lv_color_make(255, 0, 0); // 红
    }

    lv_obj_set_style_bg_color(widget->bar, color, LV_PART_INDICATOR);
}
