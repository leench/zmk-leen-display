#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zmk/ble.h>

// 连接状态枚举
enum conn_status {
    CONN_STATUS_USB_CONNECTED = 1,     // USB已连接
    CONN_STATUS_USB_DISCONNECTED = 2,  // USB未连接
    CONN_STATUS_BLE_CONNECTED = 3,     // BLE已连接
    CONN_STATUS_BLE_BONDED = 4,        // BLE已绑定但未连接
    CONN_STATUS_BLE_UNBONDED = 5       // BLE未绑定（等待配对）
};

struct connection_state {
    uint8_t status;      // 上述状态之一
    uint8_t profile_num; // BLE profile编号 (1-4)
};

struct zmk_widget_connection {
    lv_obj_t *obj;
    lv_obj_t *icon;
    lv_obj_t *label;
    lv_obj_t *profile_num;
    lv_obj_t *indicators_container;      // 指示器容器
    lv_obj_t *slider_bg;                 // 滑块背景
    lv_obj_t *profile_dots[ZMK_BLE_PROFILE_COUNT]; // profile圆点数组
    uint8_t status;
    uint8_t profile;
    sys_snode_t node;
};

// 更新profile绑定状态函数声明：
void zmk_widget_connection_update_bonding(struct zmk_widget_connection *widget, int profile_idx, bool bonded);

// 初始化控件
int zmk_widget_connection_init(struct zmk_widget_connection *widget, lv_obj_t *parent);

// 获取对象，方便 lv_obj_align
lv_obj_t *zmk_widget_connection_obj(struct zmk_widget_connection *widget);

// 更新状态
void zmk_widget_connection_update(struct zmk_widget_connection *widget,
                                  struct connection_state state);