#include "connection.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

// 全局widget链表
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

// 呼吸动画回调函数
static void anim_opa_cb(void *var, int32_t v) {
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_style_text_opa(obj, v, 0);
}

// 开始呼吸动画
static void start_breath_animation(struct zmk_widget_connection *widget) {
    if (widget->anim_running) {
        lv_anim_del(&widget->anim, NULL);
    }
    
    lv_anim_init(&widget->anim);
    lv_anim_set_var(&widget->anim, widget->icon);
    lv_anim_set_exec_cb(&widget->anim, anim_opa_cb);
    lv_anim_set_values(&widget->anim, 100, 255); // 透明度从100到255
    lv_anim_set_time(&widget->anim, 1500);       // 1.5秒周期
    lv_anim_set_repeat_count(&widget->anim, LV_ANIM_REPEAT_INFINITE); // 无限重复
    lv_anim_set_playback_time(&widget->anim, 1500); // 回放时间
    lv_anim_set_playback_delay(&widget->anim, 0);
    lv_anim_start(&widget->anim);
    widget->anim_running = true;
}

// 停止呼吸动画
static void stop_breath_animation(struct zmk_widget_connection *widget) {
    if (widget->anim_running) {
        lv_anim_del(&widget->anim, NULL);
        lv_obj_set_style_text_opa(widget->icon, LV_OPA_COVER, 0);
        widget->anim_running = false;
    }
}

// 获取连接状态
static struct connection_state get_connection_state(const zmk_event_t *_eh) {
    bool usb_ready = zmk_usb_is_hid_ready();
    bool ble_connected = zmk_ble_active_profile_is_connected();
    bool ble_bonded = !zmk_ble_active_profile_is_open();
    uint8_t ble_profile_idx = zmk_ble_active_profile_index(); // 0-3
    
    uint8_t status;
    uint8_t profile_num = 0;
    
    if (usb_ready) {
        // USB连接优先
        status = CONN_STATUS_USB;
    } else {
        // BLE模式
        profile_num = ble_profile_idx + 1; // 显示为1-4
        
        if (ble_connected) {
            status = CONN_STATUS_BLE_CONNECTED;
        } else if (ble_bonded) {
            status = CONN_STATUS_BLE_BONDED;
        } else {
            status = CONN_STATUS_BLE_UNBONDED;
        }
    }
    
    return (struct connection_state){
        .status = status,
        .profile_num = profile_num
    };
}

// 更新所有widget
static void connection_update_cb(struct connection_state state) {
    struct zmk_widget_connection *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        zmk_widget_connection_update(widget, state);
    }
}

// 创建事件监听器
ZMK_DISPLAY_WIDGET_LISTENER(widget_connection, struct connection_state,
                            connection_update_cb, get_connection_state)

// 订阅相关事件
ZMK_SUBSCRIPTION(widget_connection, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(widget_connection, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(widget_connection, zmk_usb_conn_state_changed);

int zmk_widget_connection_init(struct zmk_widget_connection *widget, lv_obj_t *parent) {
    if (!widget) return -1;

    widget->status = 0;
    widget->profile = 0;
    widget->anim_running = false;

    // 容器
    widget->obj = lv_obj_create(parent);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // 图标
    widget->icon = lv_label_create(widget->obj);
    lv_label_set_text(widget->icon, ""); // 初始无图标
    lv_obj_set_style_text_font(widget->icon, LV_FONT_DEFAULT, 0);

    // 文字 label
    widget->label = lv_label_create(widget->obj);
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, LV_FONT_DEFAULT, 0);

    // profile编号 label
    widget->profile_num = lv_label_create(widget->obj);
    lv_label_set_text(widget->profile_num, "");
    lv_obj_set_style_text_font(widget->profile_num, LV_FONT_DEFAULT, 0);

    // 水平排列：icon -> label -> profile_num
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_row(widget->obj, 0, 0); // 垂直间距
    lv_obj_set_style_pad_column(widget->obj, 5, 0); // icon 与文字间距
    lv_obj_set_style_pad_left(widget->obj, 0, 0);
    lv_obj_set_style_pad_right(widget->obj, 0, 0);

    // 添加到widget链表
    sys_slist_append(&widgets, &widget->node);
    
    // 如果是第一个widget，初始化事件监听器
    static bool listener_initialized = false;
    if (!listener_initialized) {
        widget_connection_init();
        listener_initialized = true;
        
        // 初始更新一次
        struct connection_state state = get_connection_state(NULL);
        connection_update_cb(state);
    }

    return 0;
}

lv_obj_t *zmk_widget_connection_obj(struct zmk_widget_connection *widget) {
    return widget->obj;
}

void zmk_widget_connection_update(struct zmk_widget_connection *widget,
                                  struct connection_state state) {
    if (!widget) return;

    widget->status = state.status;
    widget->profile = state.profile_num;
    
    // 停止之前的动画
    stop_breath_animation(widget);
    
    // 准备profile编号文本
    char profile_buf[8] = "";
    if (state.status >= CONN_STATUS_BLE_CONNECTED && 
        state.status <= CONN_STATUS_BLE_UNBONDED) {
        snprintf(profile_buf, sizeof(profile_buf), "%d", state.profile_num);
    }
    
    // 设置所有文本对象的字体
    lv_obj_set_style_text_font(widget->icon, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_font(widget->label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_font(widget->profile_num, &lv_font_montserrat_16, 0);
    
    switch (state.status) {
    case CONN_STATUS_USB:
        // USB连接 - 绿色
        lv_label_set_text(widget->icon, LV_SYMBOL_USB);
        lv_obj_set_style_text_color(widget->icon, lv_color_make(0, 255, 0), 0);
        lv_label_set_text(widget->label, "USB");
        lv_obj_set_style_text_color(widget->label, lv_color_make(0, 255, 0), 0);
        lv_label_set_text(widget->profile_num, "");
        break;

    case CONN_STATUS_BLE_CONNECTED:
        // BLE已连接 - 高亮蓝色
        lv_label_set_text(widget->icon, LV_SYMBOL_BLUETOOTH);
        lv_obj_set_style_text_color(widget->icon, lv_color_make(0, 150, 255), 0); // 高亮蓝色
        lv_label_set_text(widget->label, "BLE");
        lv_obj_set_style_text_color(widget->label, lv_color_make(0, 150, 255), 0);
        lv_label_set_text(widget->profile_num, profile_buf);
        lv_obj_set_style_text_color(widget->profile_num, lv_color_make(0, 150, 255), 0);
        break;

    case CONN_STATUS_BLE_BONDED:
        // BLE已绑定但未连接 - 深蓝色并呼吸闪烁
        lv_label_set_text(widget->icon, LV_SYMBOL_BLUETOOTH);
        lv_obj_set_style_text_color(widget->icon, lv_color_make(0, 0, 200), 0); // 深蓝色
        lv_label_set_text(widget->label, "BLE");
        lv_obj_set_style_text_color(widget->label, lv_color_make(0, 0, 200), 0);
        lv_label_set_text(widget->profile_num, profile_buf);
        lv_obj_set_style_text_color(widget->profile_num, lv_color_make(0, 0, 200), 0);
        
        // 开始呼吸动画
        start_breath_animation(widget);
        break;

    case CONN_STATUS_BLE_UNBONDED:
        // BLE未绑定 - 红色
        lv_label_set_text(widget->icon, LV_SYMBOL_BLUETOOTH);
        lv_obj_set_style_text_color(widget->icon, lv_color_make(255, 0, 0), 0); // 红色
        lv_label_set_text(widget->label, "BLE");
        lv_obj_set_style_text_color(widget->label, lv_color_make(255, 0, 0), 0);
        lv_label_set_text(widget->profile_num, profile_buf);
        lv_obj_set_style_text_color(widget->profile_num, lv_color_make(255, 0, 0), 0);
        break;
    }
}