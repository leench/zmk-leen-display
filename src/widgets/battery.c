// src/widgets/battery.c
#include "battery.h"

#include <lvgl.h>
#include <zephyr/logging/log.h>
#include <zmk/display.h>
#include <zmk/ble.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>

#include "fonts/lv_font_jetbrainsmono_20.h"

LOG_MODULE_REGISTER(widget_battery, LOG_LEVEL_DBG);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

// 电池状态结构体
struct battery_state {
    uint8_t source;
    uint8_t level;
};

// 电池UI对象
typedef struct {
    lv_obj_t *bar;
    lv_obj_t *label;
    lv_obj_t *nc_bar;
    lv_obj_t *nc_label;
} battery_ui_t;

static battery_ui_t battery_uis[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];

// 设置单个电池显示
static void set_battery_display(uint8_t source, uint8_t level) {
    if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) return;
    
    battery_ui_t *ui = &battery_uis[source];
    
    // 更新电池条
    if (ui->bar) {
        lv_bar_set_value(ui->bar, level, LV_ANIM_ON);
        
        if (level < 20) {
            /* 20%以下：红色系（偏暗红） */
            lv_obj_set_style_bg_color(ui->bar, lv_color_make(180, 40, 40), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(ui->bar, lv_color_make(80, 20, 20), LV_PART_MAIN);
        } else if (level < 50) {
            /* 20%-50%：保持原来的橙色系 */
            lv_obj_set_style_bg_color(ui->bar, lv_color_make(211, 144, 15), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(ui->bar, lv_color_make(110, 78, 7), LV_PART_MAIN);
        } else {
            /* 50%以上：偏白一点的灰色系 */
            lv_obj_set_style_bg_color(ui->bar, lv_color_make(200, 200, 200), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(ui->bar, lv_color_make(60, 60, 60), LV_PART_MAIN);
        }
    }
    
    // 更新标签
    if (ui->label) {
        if (level > 0) {
            lv_label_set_text_fmt(ui->label, "%d", level);
            lv_obj_set_style_text_color(ui->label, 
                level < 20 ? lv_color_hex(0xFFB802) : lv_color_hex(0xFFFFFF), 0);
            
            // 显示正常状态
            lv_obj_set_style_opa(ui->bar, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_opa(ui->label, LV_OPA_COVER, 0);
            lv_obj_set_style_opa(ui->nc_bar, LV_OPA_TRANSP, 0);
            lv_obj_set_style_opa(ui->nc_label, LV_OPA_TRANSP, 0);
        } else {
            // 未连接状态
            lv_label_set_text(ui->label, "--");
            lv_obj_set_style_text_color(ui->label, lv_color_hex(0xe63030), 0);
            
            lv_obj_set_style_opa(ui->bar, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_opa(ui->label, LV_OPA_TRANSP, 0);
            lv_obj_set_style_opa(ui->nc_bar, LV_OPA_COVER, 0);
            lv_obj_set_style_opa(ui->nc_label, LV_OPA_COVER, 0);
        }
    }
}

// 电池事件回调
static void battery_update_cb(struct battery_state state) {
    LOG_DBG("电池更新: 源=%d, 电量=%d%%", state.source, state.level);
    
    struct zmk_widget_battery *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_battery_display(state.source, state.level);
    }
}

// 从事件获取电池状态
static struct battery_state battery_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev = 
        as_zmk_peripheral_battery_state_changed(eh);
    
    if (!ev) {
        return (struct battery_state){.source = 0, .level = 0};
    }
    
    return (struct battery_state){
        .source = ev->source,
        .level = ev->state_of_charge,
    };
}

// 注册事件监听器
ZMK_DISPLAY_WIDGET_LISTENER(widget_battery, struct battery_state,
                            battery_update_cb, battery_get_state)
ZMK_SUBSCRIPTION(widget_battery, zmk_peripheral_battery_state_changed);

// 创建单个电池UI
static void create_battery_ui(lv_obj_t *parent, int index) {
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(container, 1);
    lv_obj_set_height(container, lv_pct(100));
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(container, 0, 0);

    // 正常状态电池条
    lv_obj_t *bar = lv_bar_create(container);
    lv_obj_set_size(bar, lv_pct(100), 5);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, 255, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x909090), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, 255, LV_PART_INDICATOR);
    lv_obj_set_style_anim_time(bar, 250, 0);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);

    // 正常状态标签
    lv_obj_t *label = lv_label_create(container);
    lv_obj_set_style_text_font(label, &lv_font_jetbrainsmono_20, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -3);
    lv_label_set_text(label, "--");

    // 未连接状态电池条
    lv_obj_t *nc_bar = lv_obj_create(container);
    lv_obj_set_size(nc_bar, lv_pct(100), 4);
    lv_obj_align(nc_bar, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_bg_color(nc_bar, lv_color_hex(0x9e2121), LV_PART_MAIN);
    lv_obj_set_style_radius(nc_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_opa(nc_bar, LV_OPA_TRANSP, 0);

    // 未连接状态标签
    lv_obj_t *nc_label = lv_label_create(container);
    lv_obj_set_style_text_color(nc_label, lv_color_hex(0xe63030), 0);
    lv_obj_set_style_text_font(nc_label, &lv_font_jetbrainsmono_20, 0);
    lv_obj_align(nc_label, LV_ALIGN_CENTER, 0, -3);
    lv_label_set_text(nc_label, "--");
    lv_obj_set_style_opa(nc_label, LV_OPA_TRANSP, 0);
    
    // 存储UI对象
    battery_uis[index] = (battery_ui_t){
        .bar = bar,
        .label = label,
        .nc_bar = nc_bar,
        .nc_label = nc_label,
    };
}

int zmk_widget_battery_bar_init(struct zmk_widget_battery *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    
    // 容器设置
    lv_obj_set_size(widget->obj, lv_pct(100), 46);
    lv_obj_set_layout(widget->obj, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(widget->obj, LV_FLEX_ALIGN_SPACE_EVENLY,
                         LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_column(widget->obj, 12, 0);
    lv_obj_set_style_pad_hor(widget->obj, 16, 0);
    
    // 创建电池UI
    for (int i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++) {
        create_battery_ui(widget->obj, i);
    }
    
    sys_slist_append(&widgets, &widget->node);
    widget_battery_init();
    
    return 0;
}

lv_obj_t *zmk_widget_battery_bar_obj(struct zmk_widget_battery *widget) { 
    return widget ? widget->obj : NULL; 
}

// 手动更新
void zmk_widget_battery_bar_update(struct zmk_widget_battery *widget, 
                                   uint8_t left_level, uint8_t right_level) {
    if (!widget || !widget->obj) return;
    
    set_battery_display(0, left_level);
    if (ZMK_SPLIT_BLE_PERIPHERAL_COUNT > 1) {
        set_battery_display(1, right_level);
    }
}

void zmk_widget_battery_bar_set_connected(struct zmk_widget_battery *widget,
                                          uint8_t source, bool connected) {
    if (!widget || !widget->obj || source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) return;
    
    set_battery_display(source, connected ? 50 : 0);
}