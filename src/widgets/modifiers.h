#pragma once
#include <lvgl.h>
#include <zmk/hid.h>

#define MOD_COUNT 6
#define MAX_MOD_ORDER 4  // 最多记录4个修饰键的顺序

struct zmk_widget_modifiers {
    lv_obj_t *obj;               // 父容器
    uint8_t last_mods;           // 上次的修饰键状态
    uint8_t mod_order[MAX_MOD_ORDER];  // 按键顺序记录
    uint8_t order_count;         // 当前顺序记录数量
};

// 初始化 Widget
int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget, lv_obj_t *parent);

// 更新修饰键显示
void zmk_widget_modifiers_update(struct zmk_widget_modifiers *widget);

// 获取对象
lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget);