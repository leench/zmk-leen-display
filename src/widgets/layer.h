#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>
#include "clock.h"   // 使用 zmk_widget_clock

struct zmk_widget_layer {
    lv_obj_t *obj;
    lv_obj_t *label;
    uint8_t layer;
    sys_snode_t node;
};

// 初始化 Layer Widget，对齐到时钟上方
int zmk_widget_layer_init(
    struct zmk_widget_layer *widget,
    lv_obj_t *parent,
    struct zmk_widget_clock *clock_widget
);

// 获取对象
lv_obj_t *zmk_widget_layer_obj(struct zmk_widget_layer *widget);

// 设置当前 layer（手动设置，可选）
void zmk_widget_layer_set(struct zmk_widget_layer *widget, uint8_t layer);

// 销毁 Layer Widget
void zmk_widget_layer_destroy(struct zmk_widget_layer *widget);