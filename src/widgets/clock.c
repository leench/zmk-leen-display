#include "clock.h"
#include <stdio.h>
#include <stdlib.h>  // abs()

#include "fonts/lv_font_jetbrainsmono_24.h"
#include "fonts/lv_font_jetbrainsmono_40.h"

static void clock_tick(struct k_timer *timer);
static void clock_update_display(struct zmk_widget_clock *widget);

/* =========================
 * 初始化
 * ========================= */
int zmk_widget_clock_init(struct zmk_widget_clock *widget, lv_obj_t *parent) {
    if (!widget || !parent) {
        return -EINVAL;
    }
    
    // 检查是否已初始化
    if (widget->obj) {
        return 0;
    }
    
    widget->hour = 0;
    widget->minute = 0;
    widget->second = 0;
    widget->has_sync = false;

    /* ========= 根容器 ========= */
    widget->obj = lv_obj_create(parent);
    if (!widget->obj) {
        return -ENOMEM;
    }
    
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);

    /* 让容器包裹内容 */
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    /* ========= HH:MM ========= */
    widget->label_hm = lv_label_create(widget->obj);
    if (!widget->label_hm) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        return -ENOMEM;
    }
    
    lv_label_set_text(widget->label_hm, "00:00");
    lv_obj_set_style_text_font(widget->label_hm, &lv_font_jetbrainsmono_40, 0);
    lv_obj_set_style_text_color(widget->label_hm, lv_color_white(), 0);
    lv_obj_set_size(widget->label_hm, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    /* 保持容器内顶部左对齐 */
    lv_obj_align(widget->label_hm, LV_ALIGN_TOP_LEFT, 0, 0);

    /* ========= 秒 ========= */
    widget->label_sec = lv_label_create(widget->obj);
    if (!widget->label_sec) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        widget->label_hm = NULL;
        return -ENOMEM;
    }
    
    lv_label_set_text(widget->label_sec, "00");
    lv_obj_set_style_text_font(widget->label_sec, &lv_font_jetbrainsmono_24, 0);
    lv_obj_set_style_text_color(widget->label_sec, lv_color_make(155, 155, 225), 0);
    lv_obj_set_size(widget->label_sec, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    /* 秒数相对于 HH:MM 右下对齐 */
    lv_obj_align_to(widget->label_sec, widget->label_hm,
                    LV_ALIGN_OUT_RIGHT_BOTTOM, 6, 2);

    /* ========= 根容器整体偏移 ========= */
    lv_obj_align(widget->obj, LV_ALIGN_TOP_LEFT, 0, 0);

    /* ========= 本地 1s tick ========= */
    k_timer_init(&widget->timer, clock_tick, NULL);
    k_timer_user_data_set(&widget->timer, widget);
    
    // 暂时不启动定时器，等待同步后再启动
    // k_timer_start(&widget->timer, K_SECONDS(1), K_SECONDS(1));

    return 0;
}

/* =========================
 * 显示更新
 * ========================= */
static void clock_update_display(struct zmk_widget_clock *widget) {
    if (!widget || !widget->obj || !widget->label_hm || !widget->label_sec) {
        return;
    }
    
    char buf_hm[8];
    char buf_s[4];

    snprintf(buf_hm, sizeof(buf_hm), "%02d:%02d",
             widget->hour, widget->minute);
    snprintf(buf_s, sizeof(buf_s), "%02d",
             widget->second);

    lv_label_set_text(widget->label_hm, buf_hm);
    lv_label_set_text(widget->label_sec, buf_s);

    /* 重新对齐，防止文本宽度变化 */
    lv_obj_align_to(
        widget->label_sec,
        widget->label_hm,
        LV_ALIGN_OUT_RIGHT_BOTTOM,
        6,
        2
    );
}

/* =========================
 * 每秒 tick
 * ========================= */
static void clock_tick(struct k_timer *timer) {
    struct zmk_widget_clock *widget = k_timer_user_data_get(timer);
    
    if (!widget || !widget->has_sync) {
        return;
    }

    widget->second++;
    if (widget->second >= 60) {
        widget->second = 0;
        widget->minute++;
    }
    if (widget->minute >= 60) {
        widget->minute = 0;
        widget->hour++;
    }
    if (widget->hour >= 24) {
        widget->hour = 0;
    }

    clock_update_display(widget);
}

/* =========================
 * HID 同步
 * ========================= */
void zmk_widget_clock_sync(struct zmk_widget_clock *widget,
                           uint8_t hour,
                           uint8_t minute,
                           uint8_t second,
                           int sync_threshold_s) {
    if (!widget || !widget->obj) {
        return;
    }
    
    if (!widget->has_sync) {
        widget->hour = hour;
        widget->minute = minute;
        widget->second = second;
        widget->has_sync = true;
        
        // 首次同步后启动定时器
        k_timer_start(&widget->timer, K_SECONDS(1), K_SECONDS(1));
        
        clock_update_display(widget);
        return;
    }

    int local_seconds =
        widget->hour * 3600 +
        widget->minute * 60 +
        widget->second;

    int hid_seconds =
        hour * 3600 +
        minute * 60 +
        second;

    if (abs(hid_seconds - local_seconds) > sync_threshold_s) {
        widget->hour = hour;
        widget->minute = minute;
        widget->second = second;
        clock_update_display(widget);
    }
}

lv_obj_t *zmk_widget_clock_obj(struct zmk_widget_clock *widget) {
    return widget ? widget->obj : NULL;
}

/* =========================
 * 销毁函数
 * ========================= */
void zmk_widget_clock_destroy(struct zmk_widget_clock *widget) {
    if (!widget) return;
    
    // 停止定时器
    k_timer_stop(&widget->timer);
    
    // 销毁 LVGL 对象
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
        widget->label_hm = NULL;
        widget->label_sec = NULL;
    }
    
    widget->has_sync = false;
}

/* =========================
 * 手动设置时间
 * ========================= */
void zmk_widget_clock_set_time(struct zmk_widget_clock *widget,
                               uint8_t hour,
                               uint8_t minute,
                               uint8_t second) {
    if (!widget || !widget->obj) return;
    
    widget->hour = hour % 24;
    widget->minute = minute % 60;
    widget->second = second % 60;
    widget->has_sync = true;
    
    // 如果定时器未启动，启动它
    if (!k_timer_status_get(&widget->timer)) {
        k_timer_start(&widget->timer, K_SECONDS(1), K_SECONDS(1));
    }
    
    clock_update_display(widget);
}

/* =========================
 * 获取当前时间
 * ========================= */
void zmk_widget_clock_get_time(struct zmk_widget_clock *widget,
                               uint8_t *hour,
                               uint8_t *minute,
                               uint8_t *second) {
    if (!widget || !hour || !minute || !second) return;
    
    *hour = widget->hour;
    *minute = widget->minute;
    *second = widget->second;
}