#include "modifiers.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <string.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// 字体声明
LV_FONT_DECLARE(nerd_modifiers_28);

// 全局widget引用
static struct zmk_widget_modifiers *global_widget = NULL;

// 系统类型（默认为未知）
static enum system_type current_system = SYS_UNKNOWN;

// 修饰键对应的图标（Ctrl, Shift, Alt）
static const char *mod_icons[] = {
    "󰘴",  // Ctrl
    "󰘶",  // Shift
    "󰘵",  // Alt
};

// GUI键图标（根据系统类型）
static const char *gui_icons[] = {
    "󰘳",  // SYS_UNKNOWN - 默认通用图标
    "",  // SYS_WINDOWS - Windows图标
    "󰘳",  // SYS_LINUX - Linux通用图标
    "󰘳",  // SYS_MACOS - macOS通用图标（可以改为苹果图标）
};

// 设置系统类型
void zmk_widget_modifiers_set_system_type(enum system_type type)
{
    if (type < SYS_UNKNOWN || type > SYS_MACOS) {
        type = SYS_UNKNOWN;
    }
    
    current_system = type;
    LOG_DBG("Modifiers system type set to: %d", type);
    
    // 如果widget已初始化，立即更新显示
    if (global_widget && global_widget->obj) {
        zmk_widget_modifiers_update(global_widget);
    }
}

// 获取GUI图标（根据系统类型）
static const char *get_gui_icon(void)
{
    return gui_icons[current_system];
}

// 简化版：检查修饰键变化并更新顺序
static void update_simple_order(struct zmk_widget_modifiers *widget, uint8_t current_mods)
{
    static const uint8_t mod_masks[4] = {
        (MOD_LCTL | MOD_RCTL),  // Ctrl
        (MOD_LSFT | MOD_RSFT),  // Shift
        (MOD_LALT | MOD_RALT),  // Alt
        (MOD_LGUI | MOD_RGUI),  // GUI
    };
    
    uint8_t last_mods = widget->last_mods;
    
    // 检查每个修饰键的变化
    for (int i = 0; i < 4; i++) {
        uint8_t mask = mod_masks[i];
        bool was_pressed = (last_mods & mask) != 0;
        bool is_pressed = (current_mods & mask) != 0;
        
        if (!was_pressed && is_pressed) {
            // 新按下的键，添加到顺序列表末尾
            bool already_in_list = false;
            for (int j = 0; j < widget->order_count; j++) {
                if (widget->mod_order[j] == i) {
                    already_in_list = true;
                    break;
                }
            }
            
            if (!already_in_list && widget->order_count < MAX_MOD_ORDER) {
                widget->mod_order[widget->order_count++] = i;
            }
        } else if (was_pressed && !is_pressed) {
            // 释放的键，从顺序列表中移除
            for (int j = 0; j < widget->order_count; j++) {
                if (widget->mod_order[j] == i) {
                    // 将后面的元素前移
                    for (int k = j; k < widget->order_count - 1; k++) {
                        widget->mod_order[k] = widget->mod_order[k + 1];
                    }
                    widget->order_count--;
                    break;
                }
            }
        }
    }
}

// 构建显示文本
static void build_display_text(struct zmk_widget_modifiers *widget, uint8_t current_mods, char *text, size_t text_size)
{
    int idx = 0;
    
    if (widget->extend_left) {
        // 向左延伸：先按的键在左边（正常顺序）
        for (int i = 0; i < widget->order_count; i++) {
            uint8_t mod_index = widget->mod_order[i];
            
            // 确保这个修饰键当前仍然按下
            uint8_t mask = 0;
            switch (mod_index) {
                case 0: mask = (MOD_LCTL | MOD_RCTL); break;
                case 1: mask = (MOD_LSFT | MOD_RSFT); break;
                case 2: mask = (MOD_LALT | MOD_RALT); break;
                case 3: mask = (MOD_LGUI | MOD_RGUI); break;
            }
            
            if (current_mods & mask) {
                const char *icon = NULL;
                if (mod_index == 3) {
                    // GUI键，根据系统类型选择图标
                    icon = get_gui_icon();
                } else {
                    icon = mod_icons[mod_index];
                }
                
                if (icon) {
                    idx += snprintf(&text[idx], text_size - idx, "%s", icon);
                }
            }
        }
    } else {
        // 向右延伸：先按的键在右边（反向顺序）
        for (int i = widget->order_count - 1; i >= 0; i--) {
            uint8_t mod_index = widget->mod_order[i];
            
            // 确保这个修饰键当前仍然按下
            uint8_t mask = 0;
            switch (mod_index) {
                case 0: mask = (MOD_LCTL | MOD_RCTL); break;
                case 1: mask = (MOD_LSFT | MOD_RSFT); break;
                case 2: mask = (MOD_LALT | MOD_RALT); break;
                case 3: mask = (MOD_LGUI | MOD_RGUI); break;
            }
            
            if (current_mods & mask) {
                const char *icon = NULL;
                if (mod_index == 3) {
                    // GUI键，根据系统类型选择图标
                    icon = get_gui_icon();
                } else {
                    icon = mod_icons[mod_index];
                }
                
                if (icon) {
                    idx += snprintf(&text[idx], text_size - idx, "%s", icon);
                }
            }
        }
    }
}

// 更新修饰键显示
void zmk_widget_modifiers_update(struct zmk_widget_modifiers *widget)
{
    if (!widget || !widget->obj) {
        return;
    }
    
    uint8_t current_mods = zmk_hid_get_keyboard_report()->body.modifiers;
    
    // 更新按键顺序
    update_simple_order(widget, current_mods);
    
    // 如果状态没有变化，不更新显示（但第一次初始化时需要更新）
    static bool first_update = true;
    if (!first_update && widget->last_mods == current_mods) {
        return;
    }
    
    first_update = false;
    widget->last_mods = current_mods;
    
    if (current_mods == 0) {
        // 没有修饰键按下，清空顺序记录
        widget->order_count = 0;
        
        // 隐藏修饰键标签
        lv_obj_add_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 显示修饰键标签
        lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
        
        // 构建显示文本
        char text[32] = "";
        build_display_text(widget, current_mods, text, sizeof(text));
        
        // 更新标签文本
        lv_label_set_text(widget->obj, text);
    }
}

// 设置排列方向
void zmk_widget_modifiers_set_direction(struct zmk_widget_modifiers *widget, bool extend_left)
{
    if (!widget) return;
    
    widget->extend_left = extend_left;
    LOG_DBG("Modifiers direction set to: %s", extend_left ? "LEFT" : "RIGHT");
}

// 定时器回调函数
static void modifiers_timer_cb(struct k_timer *timer)
{
    struct zmk_widget_modifiers *widget = k_timer_user_data_get(timer);
    zmk_widget_modifiers_update(widget);
}

static struct k_timer modifiers_timer;

int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget, lv_obj_t *parent)
{
    if (!widget || !parent) {
        return -EINVAL;
    }
    
    // 保存全局引用
    global_widget = widget;
    
    // 初始化结构体
    memset(widget, 0, sizeof(struct zmk_widget_modifiers));
    widget->extend_left = true;  // 默认向左延伸
    
    // 设置初始值为一个不可能的值，确保第一次更新会执行
    widget->last_mods = 0xFF;
    
    // 创建标签对象
    widget->obj = lv_label_create(parent);
    if (!widget->obj) {
        return -ENOMEM;
    }
    
    // 设置初始文本
    lv_label_set_text(widget->obj, "-");
    
    // 设置字体和颜色
    lv_obj_set_style_text_font(widget->obj, &nerd_modifiers_28, 0);
    lv_obj_set_style_text_color(widget->obj, lv_color_white(), 0);
    
    // 注意：不在 init 函数中设置任何位置
    
    // 初始化定时器
    k_timer_init(&modifiers_timer, modifiers_timer_cb, NULL);
    k_timer_user_data_set(&modifiers_timer, widget);
    k_timer_start(&modifiers_timer, K_MSEC(100), K_MSEC(100));
    
    // 立即更新一次显示状态
    zmk_widget_modifiers_update(widget);
    
    return 0;
}

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget)
{
    return widget ? widget->obj : NULL;
}