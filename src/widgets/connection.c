#include "connection.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk_widget_connection, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

#include "fonts/lv_font_symbol_16.h"
#include "fonts/lv_font_jetbrainsmono_16.h"

// 全局widget链表
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static bool listener_initialized = false;

// 获取连接状态 - 修正逻辑
static struct connection_state get_connection_state(const zmk_event_t *_eh) {
    struct zmk_endpoint_instance selected_endpoint = zmk_endpoints_selected();
    uint8_t ble_profile_idx = zmk_ble_active_profile_index();
    uint8_t status;
    uint8_t profile_num = 0;
    
    // 根据选择的端点决定显示状态
    switch (selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        // USB端点
        profile_num = 0; // USB没有profile编号
        
        if (zmk_usb_is_hid_ready()) {
            status = CONN_STATUS_USB_CONNECTED;
        } else {
            status = CONN_STATUS_USB_DISCONNECTED;
        }
        break;
        
    case ZMK_TRANSPORT_BLE:
        profile_num = ble_profile_idx + 1; // 显示为1-4
        
        // BLE状态判断（与output_status.c一致）
        if (zmk_ble_active_profile_is_connected()) {
            // 已连接
            status = CONN_STATUS_BLE_CONNECTED;
        } else if (!zmk_ble_active_profile_is_open()) {
            // 已绑定但未连接
            status = CONN_STATUS_BLE_BONDED;
        } else {
            // 未绑定（等待配对）
            status = CONN_STATUS_BLE_UNBONDED;
        }
        break;
        
    default:
        // 默认显示USB状态
        profile_num = 0;
        
        if (zmk_usb_is_hid_ready()) {
            status = CONN_STATUS_USB_CONNECTED;
        } else {
            status = CONN_STATUS_USB_DISCONNECTED;
        }
        break;
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
        if (widget && widget->obj) {
            zmk_widget_connection_update(widget, state);
        }
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
    if (!widget || !parent) return -EINVAL;
    if (widget->obj) return 0; // 已初始化

    widget->obj = lv_obj_create(parent);
    if (!widget->obj) return -ENOMEM;
    
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // 创建子对象
    widget->icon = lv_label_create(widget->obj);
    widget->label = lv_label_create(widget->obj);
    widget->profile_num = lv_label_create(widget->obj);
    
    if (!widget->icon || !widget->label || !widget->profile_num) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        return -ENOMEM;
    }

    // 基本设置
    lv_label_set_text(widget->icon, "");
    lv_obj_set_style_text_font(widget->icon, &lv_font_symbol_16, 0);
    
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &lv_font_jetbrainsmono_16, 0);
    
    lv_label_set_text(widget->profile_num, "");
    lv_obj_set_style_text_font(widget->profile_num, &lv_font_jetbrainsmono_16, 0);

    // 水平布局
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(widget->obj, 5, 0);

    // 添加到链表
    sys_slist_append(&widgets, &widget->node);
    
    // 初始化事件监听器
    if (!listener_initialized) {
        widget_connection_init();
        listener_initialized = true;
        connection_update_cb(get_connection_state(NULL));
    }

    return 0;
}

lv_obj_t *zmk_widget_connection_obj(struct zmk_widget_connection *widget) {
    return widget ? widget->obj : NULL;
}

void zmk_widget_connection_update(struct zmk_widget_connection *widget,
                                  struct connection_state state) {
    if (!widget || !widget->obj || !widget->icon || !widget->label || !widget->profile_num) {
        return;
    }

    widget->status = state.status;
    widget->profile = state.profile_num;
    
    char profile_buf[4] = "";
    
    // 根据状态设置显示
    switch (state.status) {
    case CONN_STATUS_USB_CONNECTED:
        lv_label_set_text(widget->icon, LV_SYMBOL_USB);
        lv_obj_set_style_text_color(widget->icon, lv_color_make(0, 255, 0), 0);
        lv_label_set_text(widget->label, "USB");
        lv_obj_set_style_text_color(widget->label, lv_color_make(0, 255, 0), 0);
        lv_label_set_text(widget->profile_num, "");
        break;

    case CONN_STATUS_USB_DISCONNECTED:
        lv_label_set_text(widget->icon, LV_SYMBOL_USB);
        lv_obj_set_style_text_color(widget->icon, lv_color_make(255, 0, 0), 0);
        lv_label_set_text(widget->label, "USB");
        lv_obj_set_style_text_color(widget->label, lv_color_make(255, 0, 0), 0);
        lv_label_set_text(widget->profile_num, "");
        break;

    case CONN_STATUS_BLE_CONNECTED:
        snprintf(profile_buf, sizeof(profile_buf), "%d", state.profile_num);
        lv_label_set_text(widget->icon, "\xEF\x8A\x94");
        lv_obj_set_style_text_color(widget->icon, lv_color_make(0, 0, 255), 0);
        lv_label_set_text(widget->label, "BLE");
        lv_obj_set_style_text_color(widget->label, lv_color_make(0, 0, 255), 0);
        lv_label_set_text(widget->profile_num, profile_buf);
        lv_obj_set_style_text_color(widget->profile_num, lv_color_make(0, 0, 255), 0);
        break;

    case CONN_STATUS_BLE_BONDED:
        snprintf(profile_buf, sizeof(profile_buf), "%d", state.profile_num);
        lv_label_set_text(widget->icon, "\xEF\x8A\x94");
        lv_obj_set_style_text_color(widget->icon, lv_color_make(255, 255, 255), 0);
        lv_label_set_text(widget->label, "BLE");
        lv_obj_set_style_text_color(widget->label, lv_color_make(255, 255, 255), 0);
        lv_label_set_text(widget->profile_num, profile_buf);
        lv_obj_set_style_text_color(widget->profile_num, lv_color_make(255, 255, 255), 0);
        break;

    case CONN_STATUS_BLE_UNBONDED:
        snprintf(profile_buf, sizeof(profile_buf), "%d", state.profile_num);
        lv_label_set_text(widget->icon, "\xEF\x8A\x94");
        lv_obj_set_style_text_color(widget->icon, lv_color_make(255, 0, 0), 0);
        lv_label_set_text(widget->label, "BLE");
        lv_obj_set_style_text_color(widget->label, lv_color_make(255, 0, 0), 0);
        lv_label_set_text(widget->profile_num, profile_buf);
        lv_obj_set_style_text_color(widget->profile_num, lv_color_make(255, 0, 0), 0);
        break;
        
    default:
        lv_label_set_text(widget->icon, "");
        lv_label_set_text(widget->label, "");
        lv_label_set_text(widget->profile_num, "");
        break;
    }
}

// 销毁函数
void zmk_widget_connection_destroy(struct zmk_widget_connection *widget) {
    if (!widget) return;
    
    // 从链表中移除
    struct zmk_widget_connection *prev = NULL;
    struct zmk_widget_connection *curr;
    
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, curr, node) {
        if (curr == widget) {
            if (prev) {
                sys_slist_remove(&widgets, &prev->node, &widget->node);
            } else {
                sys_slist_t *head = &widgets;
                sys_slist_remove(head, NULL, &widget->node);
            }
            break;
        }
        prev = curr;
    }
    
    // 销毁对象
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        widget->icon = NULL;
        widget->label = NULL;
        widget->profile_num = NULL;
    }
}

// 清理所有widget
void zmk_widget_connection_cleanup(void) {
    struct zmk_widget_connection *widget, *tmp;
    SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&widgets, widget, tmp, node) {
        zmk_widget_connection_destroy(widget);
    }
}

// 手动刷新
void zmk_widget_connection_refresh(struct zmk_widget_connection *widget) {
    if (!widget || !widget->obj) return;
    zmk_widget_connection_update(widget, get_connection_state(NULL));
}