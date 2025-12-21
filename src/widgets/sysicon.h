#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include "bongo_cat.h"

// 系统类型定义
enum system_type {
    SYS_UNKNOWN = 0,
    SYS_WINDOWS = 1,
    SYS_LINUX   = 2,
    SYS_MACOS   = 3,
};

struct zmk_widget_sysicon {
    lv_obj_t *obj;
    enum system_type current_system;
};

/**
 * @brief 初始化系统图标组件
 * 
 * @param widget 系统图标组件结构体
 * @param parent 父级LVGL对象
 * @param bongo_widget bongocat widget结构体指针
 * @return int 0表示成功，负数表示错误
 */
int zmk_widget_sysicon_init(struct zmk_widget_sysicon *widget, lv_obj_t *parent, 
                           struct zmk_widget_bongo_cat *bongo_widget);

/**
 * @brief 设置系统类型
 * 
 * @param widget 系统图标组件结构体
 * @param system_type 系统类型
 */
void zmk_widget_sysicon_set_system(struct zmk_widget_sysicon *widget, enum system_type system_type);

/**
 * @brief 获取系统图标对象
 * 
 * @param widget 系统图标组件结构体
 * @return lv_obj_t* LVGL对象
 */
lv_obj_t *zmk_widget_sysicon_obj(struct zmk_widget_sysicon *widget);