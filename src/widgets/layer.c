#include "layer.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/display.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/keymap.h>

#include "fonts/lv_font_montserrat_custom_24.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ============================
 *      Layer 名称映射
 * ============================ */
static const char *layer_name(uint8_t layer) {
    const char *name = zmk_keymap_layer_name(layer);
    if (name != NULL) {
        return name;
    }
    
    // 如果层没有名称，使用默认映射
    return "--";
}

/* ============================
 *      状态管理
 * ============================ */
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_state {
    uint8_t index;
    const char *label;
};

/* ============================
 *      更新层显示
 * ============================ */
static void layer_update_cb(struct layer_state state) {
    struct zmk_widget_layer *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        if (widget->label) {
            lv_label_set_text(widget->label, layer_name(state.index));
            // 可选：添加淡入效果
            lv_obj_fade_in(widget->label, 50, 0);
        }
    }
}

/* ============================
 *      获取当前层状态
 * ============================ */
static struct layer_state layer_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_state){
        .index = index,
        .label = zmk_keymap_layer_name(index)
    };
}

/* ============================
 *      事件监听器
 * ============================ */
ZMK_DISPLAY_WIDGET_LISTENER(widget_layer, struct layer_state, 
                           layer_update_cb, layer_get_state)

ZMK_SUBSCRIPTION(widget_layer, zmk_layer_state_changed);

/* ============================
 *      初始化 Layer Widget
 * ============================ */
int zmk_widget_layer_init(
    struct zmk_widget_layer *widget,
    lv_obj_t *parent,
    struct zmk_widget_clock *clock_widget
) {
    widget->layer = 0;
    
    // 容器
    widget->obj = lv_obj_create(parent);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // 文本标签
    widget->label = lv_label_create(widget->obj);
    
    // 设置初始文本
    uint8_t current_layer = zmk_keymap_highest_layer_active();
    lv_label_set_text(widget->label, layer_name(current_layer));

    // 样式设置
    lv_obj_set_style_text_color(widget->label, lv_color_make(210, 210, 210), 0);
    lv_obj_set_style_text_font(widget->label, &lv_font_montserrat_custom_24, 0);
    //lv_obj_set_style_text_font(widget->label, &lv_font_jetbrainsmono_24, 0);

    // ------------------------------
    // 宽度与时钟一致
    if (clock_widget) {
        lv_coord_t w_hm = lv_obj_get_width(clock_widget->label_hm);
        lv_coord_t w_sec = lv_obj_get_width(clock_widget->label_sec);
        lv_coord_t spacing = 5;
        lv_coord_t total_width = w_hm + w_sec + spacing;

        lv_obj_set_width(widget->label, total_width);
        lv_label_set_long_mode(widget->label, LV_LABEL_LONG_CLIP);
        lv_obj_align(widget->label, LV_ALIGN_LEFT_MID, 0, 0);

        // 对齐：时钟上方、左对齐
        lv_obj_align_to(
            widget->obj,
            clock_widget->label_hm,
            LV_ALIGN_OUT_TOP_LEFT,
            0,
            -2
        );
    } else {
        // 如果没有时钟widget，居中对齐
        lv_obj_center(widget->label);
    }

    // 添加到widget列表并初始化事件监听
    sys_slist_append(&widgets, &widget->node);
    
    // 初始化事件监听器
    widget_layer_init();

    return 0;
}

/* ============================
 *      获取对象
 * ============================ */
lv_obj_t *zmk_widget_layer_obj(struct zmk_widget_layer *widget) {
    return widget->obj;
}

/* ============================
 *      设置 Layer（手动设置，可选）
 * ============================ */
void zmk_widget_layer_set(struct zmk_widget_layer *widget, uint8_t layer) {
    if (!widget || !widget->label) return;

    widget->layer = layer;
    lv_label_set_text(widget->label, layer_name(layer));
    
    // 添加动画效果
    lv_obj_fade_in(widget->label, 100, 0);
}

/* ============================
 *      销毁 Layer Widget
 * ============================ */
void zmk_widget_layer_destroy(struct zmk_widget_layer *widget) {
    if (!widget) return;
    
    // 从widget列表中移除
    sys_slist_find_and_remove(&widgets, &widget->node);
    
    // 删除LVGL对象
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        widget->label = NULL;
    }
}