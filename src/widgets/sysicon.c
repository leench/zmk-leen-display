#include "sysicon.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// 字体声明
LV_FONT_DECLARE(nerd_modifiers_28);

// 系统图标映射
static const char *system_icons[] = {
    "-",        // SYS_UNKNOWN
    "",        // SYS_WINDOWS
    "󰌽",        // SYS_LINUX
    "󰘳",        // SYS_MACOS
};

// 系统颜色映射
static const lv_color_t system_colors[] = {
    LV_COLOR_MAKE(128, 128, 128),  // SYS_UNKNOWN (灰色)
    LV_COLOR_MAKE(0, 120, 215),    // SYS_WINDOWS (蓝色)
    LV_COLOR_MAKE(255, 69, 0),     // SYS_LINUX (橙色)
    LV_COLOR_MAKE(255, 255, 255),  // SYS_MACOS (白色)
};

static void update_sysicon_display(struct zmk_widget_sysicon *widget) {
    if (!widget || !widget->obj) {
        return;
    }

    // 获取系统图标
    const char *icon = system_icons[widget->current_system];
    lv_label_set_text(widget->obj, icon);
    
    // 设置颜色
    lv_obj_set_style_text_color(widget->obj, system_colors[widget->current_system], 0);
    
    // 显示对象
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
}

int zmk_widget_sysicon_init(struct zmk_widget_sysicon *widget, lv_obj_t *parent, 
                           struct zmk_widget_bongo_cat *bongo_widget) {
    if (!widget || !parent) {
        LOG_ERR("Invalid parameters");
        return -EINVAL;
    }

    // 初始化结构体
    widget->obj = NULL;
    widget->current_system = SYS_UNKNOWN;

    // 创建标签对象
    widget->obj = lv_label_create(parent);
    if (!widget->obj) {
        LOG_ERR("Failed to create label");
        return -ENOMEM;
    }

    // 设置字体
    lv_obj_set_style_text_font(widget->obj, &nerd_modifiers_28, 0);
    
    // 设置初始文本
    lv_label_set_text(widget->obj, system_icons[SYS_UNKNOWN]);
    lv_obj_set_style_text_color(widget->obj, system_colors[SYS_UNKNOWN], 0);

    // 对齐到bongocat左侧
    if (bongo_widget && bongo_widget->obj) {
        // 获取bongocat对象
        lv_obj_t *bongo_obj = bongo_widget->obj;
        
        // 从 bongo cat 左侧向外偏移 5px，垂直居中
        lv_obj_align_to(widget->obj, bongo_obj, LV_ALIGN_OUT_LEFT_MID, -5, 0);
        
        LOG_DBG("Sysicon aligned to bongocat left side");
    } else {
        // 如果没有bongocat对象，使用默认位置（左上角）
        lv_obj_align(widget->obj, LV_ALIGN_TOP_LEFT, 10, 10);
        LOG_WRN("No bongocat object provided, using default position");
    }

    // 初始显示
    update_sysicon_display(widget);

    return 0;
}

void zmk_widget_sysicon_set_system(struct zmk_widget_sysicon *widget, enum system_type system_type) {
    if (!widget) {
        return;
    }

    // 验证系统类型
    if (system_type < SYS_UNKNOWN || system_type > SYS_MACOS) {
        LOG_WRN("Invalid system type: %d", system_type);
        system_type = SYS_UNKNOWN;
    }

    // 如果系统类型没有变化，不更新
    if (widget->current_system == system_type) {
        return;
    }

    widget->current_system = system_type;
    update_sysicon_display(widget);
    
    LOG_DBG("System type changed to: %d", system_type);
}

lv_obj_t *zmk_widget_sysicon_obj(struct zmk_widget_sysicon *widget) {
    return widget ? widget->obj : NULL;
}