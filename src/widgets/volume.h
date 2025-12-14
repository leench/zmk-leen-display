#pragma once
#include <lvgl.h>
#include "clock.h"  // 需要包含 zmk_widget_clock 的定义

struct zmk_widget_volume {
    lv_obj_t *obj;
    lv_obj_t *bar;
    int volume;
};

// 初始化 Volume Widget，自动计算宽度并对齐到时钟下方
int zmk_widget_volume_init(struct zmk_widget_volume *widget, lv_obj_t *parent, struct zmk_widget_clock *clock_widget);

// 获取 Volume Widget 的对象
lv_obj_t *zmk_widget_volume_obj(struct zmk_widget_volume *widget);

// 设置音量值
void zmk_widget_volume_set(struct zmk_widget_volume *widget, int value);
