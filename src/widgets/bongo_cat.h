/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_bongo_cat {
    sys_snode_t node;
    lv_obj_t *obj;
};

// 初始化邦戈猫部件
int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent);

// 获取部件对象
lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget);

// 动画控制函数
void zmk_widget_bongo_cat_start_animation(struct zmk_widget_bongo_cat *widget);
void zmk_widget_bongo_cat_stop_animation(struct zmk_widget_bongo_cat *widget);
void zmk_widget_bongo_cat_set_speed(struct zmk_widget_bongo_cat *widget, uint16_t speed_ms);