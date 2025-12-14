// src/widgets/battery.c
#include "battery.h"

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/display.h>
#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/usb.h>

#include "fonts/lv_font_jetbrainsmono_20.h"

LOG_MODULE_REGISTER(widget_battery, LOG_LEVEL_DBG);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static bool initialized = false;

// 电池状态结构体
struct battery_state {
    uint8_t source;
    uint8_t level;
    bool usb_present;
};

// 电池对象存储
struct battery_object {
    lv_obj_t *bar;
    lv_obj_t *label;
    lv_obj_t *nc_bar;
    lv_obj_t *nc_label;
} battery_objects[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];

// 最后已知的电池电量
static int8_t last_battery_levels[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];

// 初始化外设跟踪
static void init_peripheral_tracking(void) {
    for (int i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++) {
        last_battery_levels[i] = -1;
    }
}

// 检测外设是否正在重连
static bool is_peripheral_reconnecting(uint8_t source, uint8_t new_level) {
    if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return false;
    }
    
    int8_t previous_level = last_battery_levels[source];
    
    bool reconnecting = (previous_level < 1) && (new_level >= 1);
    
    if (reconnecting) {
        LOG_INF("外设 %d 重连: %d%% -> %d%%", 
                source, previous_level, new_level);
    }
    
    return reconnecting;
}

// 设置电池条颜色和数值
static void set_battery_value(struct battery_state state) {
    if (state.source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        LOG_ERR("无效的外设源: %d", state.source);
        return;
    }
    
    // 检测重连
    bool reconnecting = is_peripheral_reconnecting(state.source, state.level);
    
    // 更新跟踪
    last_battery_levels[state.source] = state.level;
    
    if (reconnecting) {
        LOG_INF("外设 %d 重连，电池电量: %d%%", state.source, state.level);
    }
    
    LOG_DBG("设置电池: 源=%d, 电量=%d%%", state.source, state.level);
    
    struct battery_object *obj = &battery_objects[state.source];
    
    if (obj->bar) {
        lv_bar_set_value(obj->bar, state.level, LV_ANIM_ON);
        
        if (state.level < 20) {
            lv_obj_set_style_bg_color(obj->bar, lv_color_hex(0xD3900F), LV_PART_INDICATOR);
            lv_obj_set_style_bg_grad_color(obj->bar, lv_color_hex(0xE8AC11), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(obj->bar, lv_color_hex(0x6E4E07), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(obj->bar, lv_color_hex(0x909090), LV_PART_INDICATOR);
            lv_obj_set_style_bg_grad_color(obj->bar, lv_color_hex(0xf0f0f0), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(obj->bar, lv_color_hex(0x202020), LV_PART_MAIN);
        }
    }
    
    if (obj->label) {
        if (state.level > 0) {
            lv_label_set_text_fmt(obj->label, "%d", state.level);
            if (state.level < 20) {
                lv_obj_set_style_text_color(obj->label, lv_color_hex(0xFFB802), 0);
            } else {
                lv_obj_set_style_text_color(obj->label, lv_color_hex(0xFFFFFF), 0);
            }
            // 显示正常状态
            if (obj->bar) lv_obj_set_style_opa(obj->bar, LV_OPA_COVER, LV_PART_MAIN);
            if (obj->label) lv_obj_set_style_opa(obj->label, LV_OPA_COVER, 0);
            if (obj->nc_bar) lv_obj_set_style_opa(obj->nc_bar, LV_OPA_TRANSP, 0);
            if (obj->nc_label) lv_obj_set_style_opa(obj->nc_label, LV_OPA_TRANSP, 0);
        } else {
            // 电量 <= 0 表示未连接
            lv_obj_set_style_text_color(obj->label, lv_color_hex(0xe63030), 0);
            lv_label_set_text(obj->label, "--");
            // 显示未连接状态
            if (obj->bar) lv_obj_set_style_opa(obj->bar, LV_OPA_TRANSP, LV_PART_MAIN);
            if (obj->label) lv_obj_set_style_opa(obj->label, LV_OPA_TRANSP, 0);
            if (obj->nc_bar) lv_obj_set_style_opa(obj->nc_bar, LV_OPA_COVER, 0);
            if (obj->nc_label) lv_obj_set_style_opa(obj->nc_label, LV_OPA_COVER, 0);
        }
    }
}

// 电池事件回调
static void battery_update_cb(struct battery_state state) {
    LOG_DBG("电池更新回调: 源=%d, 电量=%d%%", state.source, state.level);
    
    struct zmk_widget_battery *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_battery_value(state);
    }
}

// 从外设电池事件获取状态
static struct battery_state peripheral_battery_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev = 
        as_zmk_peripheral_battery_state_changed(eh);
    
    if (!ev) {
        LOG_ERR("无法转换电池事件");
        return (struct battery_state){.source = 0, .level = 0};
    }
    
    LOG_DBG("收到外设电池事件: 源=%d, 电量=%d%%", ev->source, ev->state_of_charge);
    
    return (struct battery_state){
        .source = ev->source,
        .level = ev->state_of_charge,
        .usb_present = false,
    };
}

// 从中央电池事件获取状态
static struct battery_state central_battery_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    
    uint8_t level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge();
    
    LOG_DBG("收到中央电池事件: 电量=%d%%", level);
    
    return (struct battery_state) {
        .source = 0,
        .level = level,
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#else
        .usb_present = false,
#endif
    };
}

// 通用电池状态获取
static struct battery_state battery_get_state(const zmk_event_t *eh) { 
    if (as_zmk_peripheral_battery_state_changed(eh) != NULL) {
        return peripheral_battery_get_state(eh);
    } else {
        return central_battery_get_state(eh);
    }
}

// 注册事件监听器
ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_battery, struct battery_state,
                            battery_update_cb, battery_get_state)

ZMK_SUBSCRIPTION(widget_battery_battery, zmk_peripheral_battery_state_changed);

// 如果需要监听中央设备电池
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_SUBSCRIPTION(widget_battery_battery, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_battery, zmk_usb_conn_state_changed);
#endif
#endif

int zmk_widget_battery_bar_init(struct zmk_widget_battery *widget, lv_obj_t *parent) {
    if (!widget || !parent) {
        LOG_ERR("无效参数");
        return -EINVAL;
    }
    
    LOG_INF("开始初始化电池部件");
    
    widget->obj = lv_obj_create(parent);
    if (!widget->obj) {
        LOG_ERR("创建电池部件失败");
        return -ENOMEM;
    }
    
    // 设置容器尺寸和布局
    lv_obj_set_width(widget->obj, lv_pct(100));
    lv_obj_set_height(widget->obj, 46);
    
    // 使用 FLEX 布局实现水平排列
    lv_obj_set_layout(widget->obj, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(widget->obj, 
                         LV_FLEX_ALIGN_SPACE_EVENLY,
                         LV_FLEX_ALIGN_CENTER,
                         LV_FLEX_ALIGN_CENTER);
    
    // 设置容器样式
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_column(widget->obj, 12, 0);
    lv_obj_set_style_pad_bottom(widget->obj, 12, 0);
    lv_obj_set_style_pad_hor(widget->obj, 16, 0);
    
    // 外设数量
    int peripheral_count = ZMK_SPLIT_BLE_PERIPHERAL_COUNT;
    LOG_INF("创建 %d 个电池显示", peripheral_count);
    
    // 初始化电池对象数组
    for (int i = 0; i < peripheral_count; i++) {
        battery_objects[i] = (struct battery_object){0};
    }
    
    for (int i = 0; i < peripheral_count; i++) {
        // 创建电池容器
        lv_obj_t *info_container = lv_obj_create(widget->obj);
        
        // 设置容器尺寸和布局
        lv_obj_set_size(info_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(info_container, 1);
        lv_obj_set_height(info_container, lv_pct(100));
        
        // 设置容器样式
        lv_obj_set_style_bg_opa(info_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(info_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(info_container, 0, 0);

        // 正常状态电池条
        lv_obj_t *bar = lv_bar_create(info_container);
        lv_obj_set_size(bar, lv_pct(100), 5);
        lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x202020), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(bar, 255, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x909090), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(bar, 255, LV_PART_INDICATOR);
        lv_obj_set_style_bg_grad_color(bar, lv_color_hex(0xf0f0f0), LV_PART_INDICATOR);
        lv_obj_set_style_bg_dither_mode(bar, LV_DITHER_ERR_DIFF, LV_PART_INDICATOR);
        lv_obj_set_style_bg_grad_dir(bar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
        lv_obj_set_style_radius(bar, 1, LV_PART_INDICATOR);
        lv_obj_set_style_anim_time(bar, 250, 0);

        lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_opa(bar, 255, LV_PART_MAIN);
        lv_obj_set_style_opa(bar, 255, LV_PART_INDICATOR);

        // 正常状态数值显示
        lv_obj_t *label = lv_label_create(info_container);
        lv_obj_set_style_text_font(label, &lv_font_jetbrainsmono_20, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_opa(label, 255, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, -3);
        lv_label_set_text(label, "--");

        // 未连接状态电池条
        lv_obj_t *nc_bar = lv_obj_create(info_container);
        lv_obj_set_size(nc_bar, lv_pct(100), 4);
        lv_obj_align(nc_bar, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_set_style_bg_color(nc_bar, lv_color_hex(0x9e2121), LV_PART_MAIN);
        lv_obj_set_style_radius(nc_bar, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(nc_bar, 255, 0);
        lv_obj_set_style_opa(nc_bar, LV_OPA_TRANSP, 0);

        // 未连接状态符号
        lv_obj_t *nc_label = lv_label_create(info_container);
        lv_obj_set_style_text_color(nc_label, lv_color_hex(0xe63030), 0);
        lv_obj_set_style_text_font(nc_label, &lv_font_jetbrainsmono_20, 0);
        lv_obj_align(nc_label, LV_ALIGN_CENTER, 0, -3);
        lv_label_set_text(nc_label, "--");
        lv_obj_set_style_opa(nc_label, LV_OPA_TRANSP, 0);
        
        // 存储电池对象
        battery_objects[i] = (struct battery_object){
            .bar = bar,
            .label = label,
            .nc_bar = nc_bar,
            .nc_label = nc_label,
        };
        
        LOG_DBG("创建电池 %d 完成", i);
    }
    
    sys_slist_append(&widgets, &widget->node);
    
    // 初始化外设跟踪
    init_peripheral_tracking();
    
    // 初始化事件监听器
    widget_battery_battery_init();
    
    initialized = true;
    
    LOG_INF("电池部件初始化完成，高度: 46px");
    return 0;
}

lv_obj_t *zmk_widget_battery_bar_obj(struct zmk_widget_battery *widget) { 
    return widget ? widget->obj : NULL; 
}

// 手动更新电池电量（用于测试或非事件驱动场景）
void zmk_widget_battery_bar_update(struct zmk_widget_battery *widget, 
                                   uint8_t left_level, uint8_t right_level) {
    if (!widget || !widget->obj) {
        LOG_ERR("部件未初始化");
        return;
    }
    
    LOG_INF("手动更新电池电量: 左=%d%%, 右=%d%%", left_level, right_level);
    
    // 更新左电池（外设0）
    struct battery_state state_left = {
        .source = 0, 
        .level = left_level,
        .usb_present = false
    };
    set_battery_value(state_left);
    
    // 更新右电池（外设1）
    if (ZMK_SPLIT_BLE_PERIPHERAL_COUNT > 1) {
        struct battery_state state_right = {
            .source = 1, 
            .level = right_level,
            .usb_present = false
        };
        set_battery_value(state_right);
    }
}

// 手动更新连接状态（用于测试或非事件驱动场景）
void zmk_widget_battery_bar_set_connected(struct zmk_widget_battery *widget,
                                          uint8_t source, bool connected) {
    if (!widget || !widget->obj) {
        LOG_ERR("部件未初始化");
        return;
    }
    
    if (source < ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        // 通过设置电量为0来表示未连接
        struct battery_state state = {
            .source = source, 
            .level = connected ? 50 : 0, // 连接时显示50%，未连接显示0
            .usb_present = false
        };
        set_battery_value(state);
    }
}