// src/widgets/battery.h
#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_battery {
    sys_snode_t node;
    lv_obj_t *obj;
};

// 初始化电池条部件
int zmk_widget_battery_bar_init(struct zmk_widget_battery *widget, lv_obj_t *parent);

// 获取部件对象
lv_obj_t *zmk_widget_battery_bar_obj(struct zmk_widget_battery *widget);

// 更新左右电池电量（兼容旧接口）
void zmk_widget_battery_bar_update(struct zmk_widget_battery *widget, 
                                   uint8_t left_level, uint8_t right_level);

// 更新单个外设的电池电量
void zmk_widget_battery_bar_update_single(struct zmk_widget_battery *widget,
                                          uint8_t source, uint8_t level);

// 设置连接状态
void zmk_widget_battery_bar_set_connected(struct zmk_widget_battery *widget,
                                          uint8_t source, bool connected);