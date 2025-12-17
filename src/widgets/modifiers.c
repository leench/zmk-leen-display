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

// 内部位置配置
static const struct {
    int16_t pos_x;
    int16_t pos_y;
} widget_position = {
    .pos_x = -20,
    .pos_y = 12
};

// 修饰键对应的图标
static const char *mod_icons[MOD_COUNT] = {
    "󰘴",  // 0: Ctrl
    "󰘶",  // 1: Shift
    "󰘵",  // 2: Alt
    "󰌽",  // 3: GUI (Linux/Windows) System: Linux
    "",   // 4: GUI (macOS) System: Windows
    "󰘳",   // 5: GUI (默认) System: macOS
};

// 动画相关变量
static lv_anim_t color_anim;
static lv_obj_t *anim_label = NULL;
static bool is_animating = false;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

// 彩虹色关键点（使用RGB分量数组）
static const rgb_color_t rainbow_colors[] = {
    {255, 0, 0},     // 红色
    {255, 127, 0},   // 橙色
    {255, 255, 0},   // 黄色
    {0, 255, 0},     // 绿色
    {0, 255, 255},   // 青色
    {0, 0, 255},     // 蓝色
    {139, 0, 255},   // 紫色
    {255, 0, 255},   // 品红
    {255, 105, 180}, // 粉红
    {255, 0, 0},     // 回到红色
};

// 线性插值颜色
static lv_color_t lerp_color(const uint8_t r1, const uint8_t g1, const uint8_t b1,
                            const uint8_t r2, const uint8_t g2, const uint8_t b2,
                            uint16_t t, uint16_t max_t)
{
    if (t <= 0) return lv_color_make(r1, g1, b1);
    if (t >= max_t) return lv_color_make(r2, g2, b2);
    
    uint8_t r = r1 + ((r2 - r1) * t) / max_t;
    uint8_t g = g1 + ((g2 - g1) * t) / max_t;
    uint8_t b = b1 + ((b2 - b1) * t) / max_t;
    
    return lv_color_make(r, g, b);
}

// 动画回调：平滑颜色过渡
static void anim_smooth_color_cb(void *var, int32_t v)
{
    if (!var) return;
    
    // v值在0-1000之间变化，提供更精细的控制
    int total_colors = sizeof(rainbow_colors) / sizeof(rainbow_colors[0]);
    int total_segments = total_colors - 1;
    int segment_length = 1000 / total_segments;
    
    int segment = v / segment_length;
    int segment_pos = v % segment_length;
    
    if (segment >= total_segments) {
        segment = total_segments - 1;
        segment_pos = segment_length;
    }
    
    // 获取当前段落的起始和结束颜色
    const rgb_color_t *start = &rainbow_colors[segment];
    const rgb_color_t *end = &rainbow_colors[segment + 1];
    
    // 线性插值计算当前颜色
    lv_color_t current_color = lerp_color(
        start->r, start->g, start->b,
        end->r, end->g, end->b,
        segment_pos, segment_length
    );
    
    // 设置颜色
    lv_obj_set_style_text_color(var, current_color, 0);
}

// 获取当前系统对应的图标
static const char* get_system_icon(void)
{
    if (system_type == 1) {
        return mod_icons[4]; // Windows
    } else if (system_type == 2) {
        return mod_icons[3]; // Linux
    } else if (system_type == 3) {
        return mod_icons[5]; // macOS
    } else {
        return "-"; // 默认
    }
}

// 开始平滑彩虹动画
static void start_smooth_rainbow_animation(lv_obj_t *parent)
{
    if (is_animating && anim_label) {
        return;
    }
    
    // 停止之前的动画
    if (is_animating) {
        lv_anim_del(anim_label, anim_smooth_color_cb);
        lv_obj_del(anim_label);
        anim_label = NULL;
        is_animating = false;
    }
    
    // 创建动画标签对象
    anim_label = lv_label_create(parent);
    if (!anim_label) {
        return;
    }
    
    // 设置系统图标
    const char *system_icon = get_system_icon();
    lv_label_set_text(anim_label, system_icon);
    
    // 设置字体
    lv_obj_set_style_text_font(anim_label, &nerd_modifiers_28, 0);
    
    // 初始颜色（红色）
    lv_obj_set_style_text_color(anim_label, lv_color_make(255, 0, 0), 0);
    
    // 使用统一位置
    lv_obj_align(anim_label, LV_ALIGN_TOP_RIGHT, widget_position.pos_x, widget_position.pos_y);
    
    // 设置平滑彩虹动画
    lv_anim_init(&color_anim);
    lv_anim_set_var(&color_anim, anim_label);
    lv_anim_set_exec_cb(&color_anim, anim_smooth_color_cb);
    lv_anim_set_values(&color_anim, 0, 1000);  // 使用更大的范围实现更平滑的过渡
    lv_anim_set_time(&color_anim, 4000);       // 4秒完成一次完整的彩虹循环
    lv_anim_set_repeat_count(&color_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&color_anim);
    
    is_animating = true;
}

// 停止动画
static void stop_animation(void)
{
    if (is_animating && anim_label) {
        lv_anim_del(anim_label, anim_smooth_color_cb);
        lv_obj_del(anim_label);
        anim_label = NULL;
        is_animating = false;
    }
}

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
        
        // 隐藏标签
        lv_obj_add_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
        
        // 开始平滑彩虹动画
        start_smooth_rainbow_animation(lv_obj_get_parent(widget->obj));
        
    } else {
        // 停止动画
        stop_animation();
        
        // 显示标签
        lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
        
        // 构建显示文本 - 按照按键顺序
        char text[32] = "";
        int idx = 0;
        
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
                
                if (mod_index == 3) { // GUI 图标根据系统类型选择
                    // 根据系统类型动态选择图标
                    if (system_type == 1) {
                        icon = mod_icons[3]; // Windows
                    } else if (system_type == 2) {
                        icon = mod_icons[3]; // Linux (使用与Windows相同的图标)
                    } else if (system_type == 3) {
                        icon = mod_icons[4]; // macOS
                    } else {
                        icon = mod_icons[5]; // 默认
                    }
                } else if (mod_index < 3) {
                    icon = mod_icons[mod_index];
                }
                
                if (icon) {
                    idx += snprintf(&text[idx], sizeof(text) - idx, "%s", icon);
                }
            }
        }
        
        // 更新标签文本
        lv_label_set_text(widget->obj, text);
        
        // 使用统一位置重新对齐
        lv_obj_align(widget->obj, LV_ALIGN_TOP_RIGHT, widget_position.pos_x, widget_position.pos_y);
    }
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
    if (!widget) {
        return -EINVAL;
    }
    
    // 保存全局引用
    global_widget = widget;
    
    // 初始化结构体
    memset(widget, 0, sizeof(struct zmk_widget_modifiers));
    
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
    
    // 设置文本对齐方式
    lv_obj_set_style_text_align(widget->obj, LV_TEXT_ALIGN_RIGHT, 0);
    
    // 使用统一位置
    lv_obj_align(widget->obj, LV_ALIGN_TOP_RIGHT, widget_position.pos_x, widget_position.pos_y);
    
    // 初始化定时器
    k_timer_init(&modifiers_timer, modifiers_timer_cb, NULL);
    k_timer_user_data_set(&modifiers_timer, widget);
    k_timer_start(&modifiers_timer, K_MSEC(100), K_MSEC(100));
    
    // 立即更新一次显示状态
    zmk_widget_modifiers_update(widget);
    
    return 0;
}

// 添加刷新函数
void zmk_widget_modifiers_refresh(void)
{
    if (global_widget) {
        zmk_widget_modifiers_update(global_widget);
        
        // 如果当前正在显示动画，需要重启动画以使用新的系统图标
        if (is_animating && anim_label) {
            lv_obj_t *parent = lv_obj_get_parent(anim_label);
            stop_animation();
            start_smooth_rainbow_animation(parent);
        }
    }
}

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget)
{
    return widget ? widget->obj : NULL;
}