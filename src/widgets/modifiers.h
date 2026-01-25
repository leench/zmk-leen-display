#pragma once
#include <lvgl.h>
#include <zmk/hid.h>
#include "sysicon.h"

#define MOD_COUNT 4
#define MAX_MOD_ORDER 4  // 最多记录4个修饰键的顺序

struct zmk_widget_modifiers {
    lv_obj_t *obj;               // 标签对象
    uint8_t last_mods;           // 上次的修饰键状态
    uint8_t mod_order[MAX_MOD_ORDER];  // 按键顺序记录
    uint8_t order_count;         // 当前顺序记录数量
    bool extend_left;            // 排列方向：true=向左延伸，false=向右延伸
};

// 设置系统类型的函数
void zmk_widget_modifiers_set_system_type(enum system_type type);

int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget, lv_obj_t *parent);

void zmk_widget_modifiers_update(struct zmk_widget_modifiers *widget);

void zmk_widget_modifiers_set_direction(struct zmk_widget_modifiers *widget, bool extend_left);

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget);