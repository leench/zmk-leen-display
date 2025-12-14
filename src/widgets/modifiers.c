#include "modifiers.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <string.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// 字体声明
LV_FONT_DECLARE(nerd_modifiers_28);

// 修饰键对应的图标
static const char *mod_icons[MOD_COUNT] = {
    "󰘴",  // Ctrl
    "󰘶",  // Shift
    "󰘵",  // Alt
    "󰌽",  // GUI (Linux/Windows)
    "",   // GUI (macOS)
    "󰘳",   // GUI (默认)
};

// 简化版：检查修饰键变化并更新顺序
static void update_simple_order(struct zmk_widget_modifiers *widget, uint8_t current_mods)
{
    static const uint8_t mod_masks[4] = {
        (MOD_LCTL | MOD_RCTL),  // 0: Ctrl
        (MOD_LSFT | MOD_RSFT),  // 1: Shift
        (MOD_LALT | MOD_RALT),  // 2: Alt
        (MOD_LGUI | MOD_RGUI),  // 3: GUI
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

// 更新修饰键显示
void zmk_widget_modifiers_update(struct zmk_widget_modifiers *widget)
{
    if (!widget || !widget->obj) return;
    
    uint8_t current_mods = zmk_hid_get_keyboard_report()->body.modifiers;
    
    // 更新按键顺序
    update_simple_order(widget, current_mods);
    
    // 如果状态没有变化，不更新显示
    if (widget->last_mods == current_mods) {
        return;
    }
    
    widget->last_mods = current_mods;
    
    // 构建显示文本 - 按照按键顺序
    char text[32] = "";
    int idx = 0;
    
    if (current_mods == 0) {
        // 没有修饰键按下，清空顺序记录
        widget->order_count = 0;
        // 显示 "-" 字符
        idx += snprintf(&text[idx], sizeof(text) - idx, "-");
    } else {
        // 按照记录的顺序显示
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
                
                if (mod_index == 3) { // GUI 图标根据配置选择
#if CONFIG_DONGLE_SCREEN_SYSTEM_ICON == 1
                    icon = mod_icons[3]; // Linux/Windows
#elif CONFIG_DONGLE_SCREEN_SYSTEM_ICON == 2
                    icon = mod_icons[4]; // macOS
#else
                    icon = mod_icons[5]; // 默认
#endif
                } else if (mod_index < 3) {
                    icon = mod_icons[mod_index];
                }
                
                if (icon) {
                    idx += snprintf(&text[idx], sizeof(text) - idx, "%s", icon);
                }
            }
        }
    }
    
    // 更新标签文本
    lv_label_set_text(widget->obj, text);
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
    if (!widget) return -EINVAL;
    
    // 初始化结构体
    memset(widget, 0, sizeof(struct zmk_widget_modifiers));
    
    // 创建标签对象
    widget->obj = lv_label_create(parent);
    if (!widget->obj) return -ENOMEM;
    
    // 设置初始文本
    lv_label_set_text(widget->obj, "-");  // 直接设置初始文本为 "-"
    
    // 设置字体和颜色
    lv_obj_set_style_text_font(widget->obj, &nerd_modifiers_28, 0);
    lv_obj_set_style_text_color(widget->obj, lv_color_white(), 0);
    
    // 设置文本对齐方式
    lv_obj_set_style_text_align(widget->obj, LV_TEXT_ALIGN_RIGHT, 0);
    
    // 右上角对齐
    lv_obj_align(widget->obj, LV_ALIGN_TOP_RIGHT, -15, 12);
    
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