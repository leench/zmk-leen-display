// src/widgets/battery.c
#include "battery.h"

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/ble.h>

#include "fonts/lv_font_jetbrainsmono_20.h"

LOG_MODULE_REGISTER(widget_battery, LOG_LEVEL_DBG);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static bool initialized = false;

// 设置电池条颜色和数值
static void set_battery_bar_value(lv_obj_t *widget, uint8_t source, uint8_t level) {
    if (initialized && widget) {
        lv_obj_t *info_container = lv_obj_get_child(widget, source);
        if (!info_container) {
            LOG_ERR("信息容器 %d 不存在", source);
            return;
        }
        
        LOG_DBG("设置电池 %d 电量: %d%%", source, level);
        
        // 获取子对象
        lv_obj_t *bar = lv_obj_get_child(info_container, 0);
        lv_obj_t *num = lv_obj_get_child(info_container, 1);
        lv_obj_t *nc_bar = lv_obj_get_child(info_container, 2);
        lv_obj_t *nc_num = lv_obj_get_child(info_container, 3);

        if (bar) {
            lv_bar_set_value(bar, level, LV_ANIM_ON);
            
            if (level < 20) {
                lv_obj_set_style_bg_color(bar, lv_color_hex(0xD3900F), LV_PART_INDICATOR);
                lv_obj_set_style_bg_grad_color(bar, lv_color_hex(0xE8AC11), LV_PART_INDICATOR);
                lv_obj_set_style_bg_color(bar, lv_color_hex(0x6E4E07), LV_PART_MAIN);
            } else {
                lv_obj_set_style_bg_color(bar, lv_color_hex(0x909090), LV_PART_INDICATOR);
                lv_obj_set_style_bg_grad_color(bar, lv_color_hex(0xf0f0f0), LV_PART_INDICATOR);
                lv_obj_set_style_bg_color(bar, lv_color_hex(0x202020), LV_PART_MAIN);
            }
        }
        
        if (num) {
            lv_label_set_text_fmt(num, "%d", level);
            if (level < 20) {
                lv_obj_set_style_text_color(num, lv_color_hex(0xFFB802), 0);
            } else {
                lv_obj_set_style_text_color(num, lv_color_hex(0xFFFFFF), 0);
            }
        }
        
        // 确保正常状态元素可见，未连接状态隐藏
        if (bar) lv_obj_set_style_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
        if (num) lv_obj_set_style_opa(num, LV_OPA_COVER, 0);
        if (nc_bar) lv_obj_set_style_opa(nc_bar, LV_OPA_TRANSP, 0);
        if (nc_num) lv_obj_set_style_opa(nc_num, LV_OPA_TRANSP, 0);
    }
}

// 设置连接状态显示
static void set_battery_bar_connected(lv_obj_t *widget, uint8_t source, bool connected) {
    if (initialized && widget) {
        lv_obj_t *info_container = lv_obj_get_child(widget, source);
        if (!info_container) return;
        
        lv_obj_t *bar = lv_obj_get_child(info_container, 0);
        lv_obj_t *num = lv_obj_get_child(info_container, 1);
        lv_obj_t *nc_bar = lv_obj_get_child(info_container, 2);
        lv_obj_t *nc_num = lv_obj_get_child(info_container, 3);

        LOG_DBG("外设 %d %s", source,
                connected ? "已连接" : "未连接");

        if (connected) {
            if (nc_bar) lv_obj_set_style_opa(nc_bar, LV_OPA_TRANSP, 0);
            if (nc_num) lv_obj_set_style_opa(nc_num, LV_OPA_TRANSP, 0);
            if (bar) lv_obj_set_style_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
            if (num) lv_obj_set_style_opa(num, LV_OPA_COVER, 0);
        } else {
            if (bar) lv_obj_set_style_opa(bar, LV_OPA_TRANSP, LV_PART_MAIN);
            if (num) lv_obj_set_style_opa(num, LV_OPA_TRANSP, 0);
            if (nc_bar) lv_obj_set_style_opa(nc_bar, LV_OPA_COVER, 0);
            if (nc_num) lv_obj_set_style_opa(nc_num, LV_OPA_COVER, 0);
        }
    }
}

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
    
    // 设置容器尺寸和布局 - 使用 FLEX 布局
    lv_obj_set_width(widget->obj, lv_pct(100));
    lv_obj_set_height(widget->obj, 46); // 固定高度
    
    // 使用 FLEX 布局实现水平排列
    lv_obj_set_layout(widget->obj, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(widget->obj, 
                         LV_FLEX_ALIGN_SPACE_EVENLY,  // 主轴对齐
                         LV_FLEX_ALIGN_CENTER,        // 交叉轴对齐
                         LV_FLEX_ALIGN_CENTER);       // 轨道对齐
    
    // 设置容器样式
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_column(widget->obj, 12, 0);
    lv_obj_set_style_pad_bottom(widget->obj, 12, 0);
    lv_obj_set_style_pad_hor(widget->obj, 16, 0);
    
    // 外设数量
    int peripheral_count = ZMK_SPLIT_BLE_PERIPHERAL_COUNT;
    LOG_INF("创建 %d 个电池显示", peripheral_count);
    
    for (int i = 0; i < peripheral_count; i++) {
        // 创建电池容器
        lv_obj_t *info_container = lv_obj_create(widget->obj);
        
        // 设置容器尺寸和布局
        lv_obj_set_size(info_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(info_container, 1); // 等分宽度
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
        lv_obj_set_style_opa(bar, 255, LV_PART_MAIN); // 改为可见
        lv_obj_set_style_opa(bar, 255, LV_PART_INDICATOR);

        // 正常状态数值显示 - 使用你的字体
        lv_obj_t *num = lv_label_create(info_container);
        lv_obj_set_style_text_font(num, &lv_font_jetbrainsmono_20, 0);
        lv_obj_set_style_text_color(num, lv_color_white(), 0);
        lv_obj_set_style_opa(num, 255, 0);
        lv_obj_align(num, LV_ALIGN_CENTER, 0, -3);
        lv_label_set_text(num, "N/A");

        // 未连接状态电池条
        lv_obj_t *nc_bar = lv_obj_create(info_container);
        lv_obj_set_size(nc_bar, lv_pct(100), 4);
        lv_obj_align(nc_bar, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_set_style_bg_color(nc_bar, lv_color_hex(0x9e2121), LV_PART_MAIN);
        lv_obj_set_style_radius(nc_bar, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(nc_bar, 255, 0);
        lv_obj_set_style_opa(nc_bar, LV_OPA_TRANSP, 0); // 初始隐藏

        // 未连接状态符号
        lv_obj_t *nc_num = lv_label_create(info_container);
        lv_obj_set_style_text_color(nc_num, lv_color_hex(0xe63030), 0);
        lv_obj_set_style_text_font(nc_num, &lv_font_jetbrainsmono_20, 0);
        lv_obj_align(nc_num, LV_ALIGN_CENTER, 0, -3);
        lv_label_set_text(nc_num, "--"); // 使用 X 代替 LV_SYMBOL_CLOSE
        lv_obj_set_style_opa(nc_num, LV_OPA_TRANSP, 0); // 初始隐藏
        
        LOG_DBG("创建电池 %d 完成", i);
    }
    
    sys_slist_append(&widgets, &widget->node);
    initialized = true;
    
    LOG_INF("电池部件初始化完成，高度: 40px");
    return 0;
}

lv_obj_t *zmk_widget_battery_bar_obj(struct zmk_widget_battery *widget) { 
    return widget ? widget->obj : NULL; 
}

// 手动更新电池电量
void zmk_widget_battery_bar_update(struct zmk_widget_battery *widget, 
                                   uint8_t left_level, uint8_t right_level) {
    if (!widget || !widget->obj) {
        LOG_ERR("部件未初始化");
        return;
    }
    
    LOG_INF("更新电池电量: 左=%d%%, 右=%d%%", left_level, right_level);
    
    // 更新左电池（外设0）
    set_battery_bar_value(widget->obj, 0, left_level);
    
    // 更新右电池（外设1）
    set_battery_bar_value(widget->obj, 1, right_level);
}

// 更新单个外设的电池电量
void zmk_widget_battery_bar_update_single(struct zmk_widget_battery *widget,
                                          uint8_t source, uint8_t level) {
    if (!widget || !widget->obj) {
        LOG_ERR("部件未初始化");
        return;
    }
    
    if (source < 2) { // 硬编码为2
        set_battery_bar_value(widget->obj, source, level);
    }
}

// 设置连接状态
void zmk_widget_battery_bar_set_connected(struct zmk_widget_battery *widget,
                                          uint8_t source, bool connected) {
    if (!widget || !widget->obj) {
        LOG_ERR("部件未初始化");
        return;
    }
    
    if (source < 2) { // 硬编码为2
        set_battery_bar_connected(widget->obj, source, connected);
    }
}