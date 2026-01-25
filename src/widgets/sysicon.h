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

int zmk_widget_sysicon_init(struct zmk_widget_sysicon *widget, lv_obj_t *parent, 
                           struct zmk_widget_bongo_cat *bongo_widget);

void zmk_widget_sysicon_set_system(struct zmk_widget_sysicon *widget, enum system_type system_type);

lv_obj_t *zmk_widget_sysicon_obj(struct zmk_widget_sysicon *widget);