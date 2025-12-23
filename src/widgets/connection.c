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

// profile count
int total_profile_slots = ZMK_BLE_PROFILE_COUNT;

// 全局widget链表
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static bool listener_initialized = false;

// 视觉常量定义 - 方便后续调整
#define INDICATOR_DOT_SIZE 8                     // 圆点大小
#define INDICATOR_DOT_SPACING 16                  // 圆点间距(像素)
#define INDICATOR_DOT_WIDTH INDICATOR_DOT_SPACING // 每个圆点占用的宽度
#define INDICATOR_BONDED_COLOR lv_color_make(0, 180, 255)   // 亮蓝色：RGB(0, 180, 255)
#define INDICATOR_UNBONDED_COLOR lv_color_make(255, 140, 0) // 亮橙色：RGB(255, 140, 0)
#define INDICATOR_UNSELECTED_OPA LV_OPA_50        // 未选中圆点透明度
#define INDICATOR_SELECTED_OPA LV_OPA_100         // 选中圆点透明度
#define INDICATOR_SELECTED_BORDER_COLOR lv_color_make(200, 200, 200) // 选中圆点边框颜色
#define INDICATOR_SELECTED_BORDER_WIDTH 4                           // 选中圆点边框宽度


// 获取连接状态 - 严格按照transport类型处理
static struct connection_state get_connection_state(const zmk_event_t *_eh) {
    struct zmk_endpoint_instance selected_endpoint = zmk_endpoints_selected();
    uint8_t ble_profile_idx = zmk_ble_active_profile_index();
    uint8_t status = CONN_STATUS_USB_DISCONNECTED; // 默认值
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
        
        // BLE状态判断
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


// 修改 update_profile_indicators 函数
static void update_profile_indicators(struct zmk_widget_connection *widget) {
    if (!widget || !widget->indicators_container) return;
    
    // 总是获取当前激活的BLE profile索引
    uint8_t current_profile = zmk_ble_active_profile_index();
    
    // 获取所有profile的绑定状态
    uint32_t profile_bonded_mask = 0;
    for (int i = 0; i < total_profile_slots; i++) {
        // 如果该 Profile 不是 "Open" 状态，说明已经 Bonded（已绑定）
        if (!zmk_ble_profile_is_open(i)) {
            profile_bonded_mask |= BIT(i);
        }
    }
    
    // 更新所有圆点
    for (int i = 0; i < total_profile_slots; i++) {
        if (!widget->profile_dots[i]) continue;
        
        // 判断是否已绑定
        bool is_bonded = (profile_bonded_mask & BIT(i)) != 0;
        
        // 设置颜色
        lv_color_t color = is_bonded ? 
            INDICATOR_BONDED_COLOR : 
            INDICATOR_UNBONDED_COLOR;
        
        // 设置透明度：当前选中的profile完全显示，其他变暗
        lv_opa_t opa = (i == current_profile) ? 
            INDICATOR_SELECTED_OPA : 
            INDICATOR_UNSELECTED_OPA;
        
        lv_obj_set_style_bg_color(widget->profile_dots[i], color, 0);
        lv_obj_set_style_bg_opa(widget->profile_dots[i], opa, 0);
        
        // 如果是当前选中的圆点，添加边框作为背景效果
        // if (i == current_profile) {
        //     lv_obj_set_style_border_opa(widget->profile_dots[i], LV_OPA_100, 0);
        // } else {
        //     // 非选中圆点移除边框
        //     lv_obj_set_style_border_width(widget->profile_dots[i], 0, 0);
        // }
    }
}

int zmk_widget_connection_init(struct zmk_widget_connection *widget, lv_obj_t *parent) {
    if (!widget || !parent) return -EINVAL;
    if (widget->obj) return 0; // 已初始化

    // 创建主容器（垂直布局：第一行是原有内容，第二行是指示器）
    widget->obj = lv_obj_create(parent);
    if (!widget->obj) return -ENOMEM;
    
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    
    // 设置垂直布局
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(widget->obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(widget->obj, 2, 0); // 恢复较小的行间距

    // 创建第一行容器（原有内容：图标、标签、profile编号）
    lv_obj_t *first_row = lv_obj_create(widget->obj);
    if (!first_row) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        return -ENOMEM;
    }
    
    lv_obj_clear_flag(first_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(first_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(first_row, 0, 0);
    lv_obj_set_size(first_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    
    // 第一行水平布局
    lv_obj_set_flex_flow(first_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(first_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(first_row, 5, 0);

    // 在第一行中创建原有子对象
    widget->icon = lv_label_create(first_row);
    widget->label = lv_label_create(first_row);
    widget->profile_num = lv_label_create(first_row);
    
    if (!widget->icon || !widget->label || !widget->profile_num) {
        lv_obj_del(first_row);
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        return -ENOMEM;
    }

    // 基本设置（保持原有逻辑）
    lv_label_set_text(widget->icon, "");
    lv_obj_set_style_text_font(widget->icon, &lv_font_symbol_16, 0);
    
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &lv_font_jetbrainsmono_16, 0);
    
    lv_label_set_text(widget->profile_num, "");
    lv_obj_set_style_text_font(widget->profile_num, &lv_font_jetbrainsmono_16, 0);

    // 创建第二行：profile指示器容器
    widget->indicators_container = lv_obj_create(widget->obj);
    if (!widget->indicators_container) {
        lv_obj_del(first_row);
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        return -ENOMEM;
    }
    
    // 设置指示器容器样式
    lv_obj_clear_flag(widget->indicators_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->indicators_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->indicators_container, 0, 0);
    lv_obj_set_size(widget->indicators_container, 
                    total_profile_slots * INDICATOR_DOT_WIDTH, 
                    INDICATOR_DOT_SIZE + 4 + 12); // 高度略大于圆点大小
    
    // 给指示器容器增加上边距，让它往下一点
    lv_obj_set_style_pad_top(widget->indicators_container, 12, 0);
    
    // 创建profile圆点（使用lv_obj替代字符）
    for (int i = 0; i < total_profile_slots; i++) {
        widget->profile_dots[i] = lv_obj_create(widget->indicators_container);
        if (!widget->profile_dots[i]) {
            // 创建失败，清理已创建的对象
            for (int j = 0; j < i; j++) {
                if (widget->profile_dots[j]) {
                    lv_obj_del(widget->profile_dots[j]);
                }
            }
            lv_obj_del(widget->indicators_container);
            lv_obj_del(first_row);
            lv_obj_del(widget->obj);
            widget->obj = NULL;
            return -ENOMEM;
        }
        
        // 设置圆点样式
        lv_obj_set_size(widget->profile_dots[i], INDICATOR_DOT_SIZE, INDICATOR_DOT_SIZE);
        lv_obj_set_style_radius(widget->profile_dots[i], INDICATOR_DOT_SIZE / 2, 0); // 圆形
        lv_obj_set_style_bg_opa(widget->profile_dots[i], INDICATOR_UNSELECTED_OPA, 0);
        lv_obj_set_style_border_width(widget->profile_dots[i], 0, 0);
        lv_obj_set_style_pad_all(widget->profile_dots[i], 0, 0);
        
        // 设置位置 - 居中
        int dot_x = i * INDICATOR_DOT_WIDTH + 
                    (INDICATOR_DOT_WIDTH - INDICATOR_DOT_SIZE) / 2;
        int dot_y = (lv_obj_get_height(widget->indicators_container) - INDICATOR_DOT_SIZE) / 2;
        lv_obj_set_pos(widget->profile_dots[i], dot_x, dot_y);
    }

    // 添加到链表
    sys_slist_append(&widgets, &widget->node);
    
    // 初始化事件监听器
    if (!listener_initialized) {
        widget_connection_init();
        listener_initialized = true;
        connection_update_cb(get_connection_state(NULL));
    }
    
    // 初始更新profile指示器
    update_profile_indicators(widget);

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
    
    // 根据状态设置显示（保持原有逻辑）
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
    
    // 总是更新profile指示器状态
    // 因为连接状态变化时，绑定状态可能也发生了变化
    update_profile_indicators(widget);
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
    
    // 销毁指示器相关对象
    if (widget->indicators_container) {
        // 销毁所有圆点对象
        for (int i = 0; i < total_profile_slots; i++) {
            if (widget->profile_dots[i]) {
                lv_obj_del(widget->profile_dots[i]);
                widget->profile_dots[i] = NULL;
            }
        }
        
        // 销毁指示器容器
        lv_obj_del(widget->indicators_container);
        widget->indicators_container = NULL;
    }
    
    // 销毁主对象（会自动销毁第一行容器及其子对象）
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
    
    // 更新主连接状态
    zmk_widget_connection_update(widget, get_connection_state(NULL));
    
    // 更新profile指示器
    update_profile_indicators(widget);
}

// 更新profile绑定状态（供外部调用）
void zmk_widget_connection_update_bonding(struct zmk_widget_connection *widget, int profile_idx, bool bonded) {
    if (!widget || !widget->indicators_container || profile_idx < 0 || profile_idx >= total_profile_slots) {
        return;
    }
    
    if (widget->profile_dots[profile_idx]) {
        // 更新颜色
        lv_color_t color = bonded ? 
            INDICATOR_BONDED_COLOR : 
            INDICATOR_UNBONDED_COLOR;
        
        lv_obj_set_style_bg_color(widget->profile_dots[profile_idx], color, 0);
        
        // 更新透明度（如果是当前选中的profile）
        uint8_t current_profile = zmk_ble_active_profile_index();
        if (profile_idx == current_profile) {
            lv_obj_set_style_bg_opa(widget->profile_dots[profile_idx], INDICATOR_SELECTED_OPA, 0);
        }
    }
}