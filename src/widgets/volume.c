#include "volume.h"
#include <lvgl.h>
#include <zephyr/kernel.h>

#define VOLUME_MIN 0
#define VOLUME_MAX 100
#define VOLUME_DEFAULT 100
#define BAR_HEIGHT 4
#define CLOCK_SPACING 5
#define ANIM_DURATION 300
#define ALIGN_SPACING 5

static const lv_color_t COLOR_GRAY = LV_COLOR_MAKE(0x50, 0x50, 0x50);

int zmk_widget_volume_init(struct zmk_widget_volume *widget, lv_obj_t *parent, 
                          struct zmk_widget_clock *clock_widget) {
    if (!widget || !parent || !clock_widget) {
        return -EINVAL;
    }

    widget->volume = VOLUME_DEFAULT;

    widget->obj = lv_obj_create(parent);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    widget->bar = lv_bar_create(widget->obj);
    lv_bar_set_range(widget->bar, VOLUME_MIN, VOLUME_MAX);
    lv_bar_set_value(widget->bar, widget->volume, LV_ANIM_OFF);

    // 设置初始颜色（红色满条）
    lv_obj_set_style_bg_color(widget->bar, lv_color_make(255, 0, 0), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(widget->bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(widget->bar, COLOR_GRAY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->bar, LV_OPA_COVER, LV_PART_MAIN);

    lv_coord_t width = lv_obj_get_width(clock_widget->label_hm) + 
                      lv_obj_get_width(clock_widget->label_sec) + 
                      CLOCK_SPACING;
    
    lv_obj_set_size(widget->bar, width, BAR_HEIGHT);
    lv_obj_align_to(widget->obj, clock_widget->label_hm, LV_ALIGN_OUT_BOTTOM_LEFT, 0, ALIGN_SPACING);

    return 0;
}

lv_obj_t *zmk_widget_volume_obj(struct zmk_widget_volume *widget) {
    return widget ? widget->obj : NULL;
}

static lv_color_t calculate_color(int volume) {
    lv_color_t color;
    
    if (volume <= 25) {
        color = lv_color_make(0, 255, 0); // 绿
    } else if (volume <= 30) {
        // 25~30 渐变 绿→黄
        uint8_t t = (volume - 25) * 255 / 5;
        color = lv_color_make(t, 255, 0);
    } else if (volume <= 65) {
        color = lv_color_make(255, 255, 0); // 黄
    } else if (volume <= 70) {
        // 65~70 渐变 黄→红
        uint8_t t = (volume - 65) * 255 / 5;
        color = lv_color_make(255, 255 - t, 0);
    } else {
        color = lv_color_make(255, 0, 0); // 红
    }
    
    return color;
}

void zmk_widget_volume_set(struct zmk_widget_volume *widget, int value) {
    if (!widget) return;

    if (value < VOLUME_MIN) value = VOLUME_MIN;
    if (value > VOLUME_MAX) value = VOLUME_MAX;

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, widget->bar);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_bar_set_value);
    lv_anim_set_values(&anim, widget->volume, value);
    lv_anim_set_time(&anim, ANIM_DURATION);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_start(&anim);

    widget->volume = value;
    lv_obj_set_style_bg_color(widget->bar, calculate_color(value), LV_PART_INDICATOR);
}