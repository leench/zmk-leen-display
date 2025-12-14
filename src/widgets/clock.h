#pragma once
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <stdint.h>

struct zmk_widget_clock {
    lv_obj_t *obj;
    lv_obj_t *label_hm;
    lv_obj_t *label_sec;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool has_sync;

    struct k_timer timer;
};

/* 初始化时钟 widget，可整体偏移 */
int zmk_widget_clock_init(struct zmk_widget_clock *widget, lv_obj_t *parent);

/* 获取时钟根对象 */
lv_obj_t *zmk_widget_clock_obj(struct zmk_widget_clock *widget);

/* HID 同步时钟 */
void zmk_widget_clock_sync(struct zmk_widget_clock *widget,
                           uint8_t hour,
                           uint8_t minute,
                           uint8_t second,
                           int sync_threshold_s);
