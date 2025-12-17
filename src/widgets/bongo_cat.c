#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
LOG_MODULE_REGISTER(bongo_cat_widget, LOG_LEVEL_DBG);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>
#include <lvgl.h>

#include "bongo_cat.h"

/* ================= 图片资源 ================= */

LV_IMG_DECLARE(bongo_cat_none);
LV_IMG_DECLARE(bongo_cat_left1);
LV_IMG_DECLARE(bongo_cat_left2);
LV_IMG_DECLARE(bongo_cat_right1);
LV_IMG_DECLARE(bongo_cat_right2);
LV_IMG_DECLARE(bongo_cat_both1);
LV_IMG_DECLARE(bongo_cat_both1_open);
LV_IMG_DECLARE(bongo_cat_both2);

/* ================= 动画速度 ================= */

#define ANIMATION_SPEED_IDLE 10000
#define ANIMATION_SPEED_SLOW 2000
#define ANIMATION_SPEED_MID  500
#define ANIMATION_SPEED_FAST 200

/* ================= 动画帧 ================= */

static const lv_img_dsc_t *idle_imgs[] = {
    &bongo_cat_both1_open,
    &bongo_cat_both1_open,
    &bongo_cat_both1_open,
    &bongo_cat_both1,
};

static const lv_img_dsc_t *slow_imgs[] = {
    &bongo_cat_left1,
    &bongo_cat_both1,
    &bongo_cat_both1,
    &bongo_cat_right1,
    &bongo_cat_both1,
    &bongo_cat_both1,
    &bongo_cat_left1,
    &bongo_cat_both1,
    &bongo_cat_both1,
};

static const lv_img_dsc_t *mid_imgs[] = {
    &bongo_cat_left2,
    &bongo_cat_left1,
    &bongo_cat_none,
    &bongo_cat_right2,
    &bongo_cat_right1,
    &bongo_cat_none,
};

static const lv_img_dsc_t *fast_imgs[] = {
    &bongo_cat_both2,
    &bongo_cat_both1,
    &bongo_cat_none,
    &bongo_cat_none,
};

#define SRC(imgs) (const void **)imgs, sizeof(imgs) / sizeof(lv_img_dsc_t *)

/* ================= 炫彩颜色表（FAST 专用） ================= */

static const lv_color_t fast_colors[] = {
    LV_COLOR_MAKE(255,   0,   0),
    LV_COLOR_MAKE(255, 128,   0),
    LV_COLOR_MAKE(255, 255,   0),
    LV_COLOR_MAKE(0,   255,   0),
    LV_COLOR_MAKE(0,   255, 255),
    LV_COLOR_MAKE(0,     0, 255),
    LV_COLOR_MAKE(180,   0, 255),
};

#define FAST_COLOR_COUNT (sizeof(fast_colors) / sizeof(fast_colors[0]))

/* ================= widget 管理 ================= */

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct bongo_cat_wpm_status_state {
    uint8_t wpm;
};

static enum anim_state current_anim_state = anim_state_none;

/* ================= 炫彩动画 ================= */

static lv_anim_t fast_color_anim;

static void fast_color_anim_cb(void *obj, int32_t v) {
    lv_obj_t *o = (lv_obj_t*)obj;
    uint32_t idx = v % FAST_COLOR_COUNT;
    lv_obj_set_style_img_recolor(o, fast_colors[idx], 0);
    lv_obj_set_style_img_recolor_opa(o, LV_OPA_COVER, 0);
}


static void start_fast_color_anim(lv_obj_t *obj) {
    lv_anim_del(obj, fast_color_anim_cb);

    lv_anim_init(&fast_color_anim);
    lv_anim_set_var(&fast_color_anim, obj);
    lv_anim_set_exec_cb(&fast_color_anim, fast_color_anim_cb);
    lv_anim_set_values(&fast_color_anim, 0, FAST_COLOR_COUNT - 1); // ✔ 从0到数量-1
    lv_anim_set_time(&fast_color_anim, 200);  // 每帧切换间隔
    lv_anim_set_repeat_count(&fast_color_anim, LV_ANIM_REPEAT_INFINITE); // ✔ 无限循环
    lv_anim_start(&fast_color_anim);
}

static void stop_fast_color_anim(lv_obj_t *obj) {
    lv_anim_del(obj, fast_color_anim_cb);
    lv_obj_set_style_img_recolor_opa(obj, LV_OPA_0, 0);
}

/* ================= 动画切换逻辑 ================= */

static void set_animation(lv_obj_t *anim_obj,
                          struct bongo_cat_wpm_status_state state) {

    /* 每次切换前清理炫彩 */
    stop_fast_color_anim(anim_obj);

    if (state.wpm < 5) {
        if (current_anim_state != anim_state_idle) {
            lv_animimg_set_src(anim_obj, SRC(idle_imgs));
            lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_IDLE);
            lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(anim_obj);
            current_anim_state = anim_state_idle;
        }

    } else if (state.wpm < 25) {
        if (current_anim_state != anim_state_slow) {
            lv_animimg_set_src(anim_obj, SRC(slow_imgs));
            lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_SLOW);
            lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(anim_obj);
            current_anim_state = anim_state_slow;
        }

    } else if (state.wpm < 50) {
        if (current_anim_state != anim_state_mid) {
            lv_animimg_set_src(anim_obj, SRC(mid_imgs));
            lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_MID);
            lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(anim_obj);
            current_anim_state = anim_state_mid;
        }

    } else {
        if (current_anim_state != anim_state_fast) {
            lv_animimg_set_src(anim_obj, SRC(fast_imgs));
            lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_FAST);
            lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(anim_obj);

            /* ⭐ FAST 启动炫彩 */
            start_fast_color_anim(anim_obj);

            current_anim_state = anim_state_fast;
        }
    }
}

/* ================= ZMK 事件 ================= */

struct bongo_cat_wpm_status_state
bongo_cat_wpm_status_get_state(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    return (struct bongo_cat_wpm_status_state){ .wpm = ev->state };
}

void bongo_cat_wpm_status_update_cb(struct bongo_cat_wpm_status_state state) {
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_animation(widget->obj, state);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(
    widget_bongo_cat,
    struct bongo_cat_wpm_status_state,
    bongo_cat_wpm_status_update_cb,
    bongo_cat_wpm_status_get_state
)

ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

/* ================= 初始化 ================= */

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget,
                              lv_obj_t *parent) {
    widget->obj = lv_animimg_create(parent);
    lv_obj_center(widget->obj);

    sys_slist_append(&widgets, &widget->node);
    widget_bongo_cat_init();

    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) {
    return widget ? widget->obj : NULL;
}

void zmk_widget_bongo_cat_set_wpm(struct zmk_widget_bongo_cat *widget,
                                  uint8_t wpm) {
    struct bongo_cat_wpm_status_state state = {.wpm = wpm};
    set_animation(widget->obj, state);
}
