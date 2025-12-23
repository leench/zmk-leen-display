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

/* ================= 炫彩动画 ================= */

static void fast_color_anim_cb(void *obj, int32_t v) {
    lv_obj_t *o = (lv_obj_t*)obj;
    if (!o || !lv_obj_is_valid(o)) return;
    
    uint32_t idx = v % FAST_COLOR_COUNT;
    lv_obj_set_style_img_recolor(o, fast_colors[idx], 0);
    lv_obj_set_style_img_recolor_opa(o, LV_OPA_COVER, 0);
}

static void start_fast_color_anim(lv_obj_t *obj) {
    if (!obj || !lv_obj_is_valid(obj)) return;
    
    lv_anim_del(obj, fast_color_anim_cb);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, fast_color_anim_cb);
    lv_anim_set_values(&anim, 0, FAST_COLOR_COUNT * 10 - 1); // 循环10次
    lv_anim_set_time(&anim, 200 * FAST_COLOR_COUNT * 10); // 10次完整循环的时间
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim);
}

static void stop_fast_color_anim(lv_obj_t *obj) {
    if (!obj) return;
    lv_anim_del(obj, fast_color_anim_cb);
    lv_obj_set_style_img_recolor_opa(obj, LV_OPA_0, 0);
}

/* ================= 动画切换逻辑 ================= */

static void set_animation(lv_obj_t *anim_obj,
                          struct bongo_cat_wpm_status_state state) {
    if (!anim_obj || !lv_obj_is_valid(anim_obj)) return;

    /* 停止当前动画 */
    stop_fast_color_anim(anim_obj);

    if (state.wpm < 5) {
        lv_animimg_set_src(anim_obj, SRC(idle_imgs));
        lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_IDLE);
        lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(anim_obj);

    } else if (state.wpm < 30) {
        lv_animimg_set_src(anim_obj, SRC(slow_imgs));
        lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_SLOW);
        lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(anim_obj);

    } else if (state.wpm < 75) {
        lv_animimg_set_src(anim_obj, SRC(mid_imgs));
        lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_MID);
        lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(anim_obj);

    } else {
        lv_animimg_set_src(anim_obj, SRC(fast_imgs));
        lv_animimg_set_duration(anim_obj, ANIMATION_SPEED_FAST);
        lv_animimg_set_repeat_count(anim_obj, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(anim_obj);

        /* FAST 模式启动炫彩动画 */
        start_fast_color_anim(anim_obj);
    }
}

/* ================= ZMK 事件 ================= */

struct bongo_cat_wpm_status_state
bongo_cat_wpm_status_get_state(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    if (!ev) {
        return (struct bongo_cat_wpm_status_state){ .wpm = 0 };
    }
    return (struct bongo_cat_wpm_status_state){ .wpm = ev->state };
}

void bongo_cat_wpm_status_update_cb(struct bongo_cat_wpm_status_state state) {
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        if (widget && widget->obj) {
            set_animation(widget->obj, state);
        }
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
    if (!widget || !parent) {
        LOG_ERR("无效参数");
        return -EINVAL;
    }
    
    // 检查是否已初始化
    if (widget->obj) {
        LOG_WRN("bongo cat widget 已初始化");
        return 0;
    }
    
    widget->obj = lv_animimg_create(parent);
    if (!widget->obj) {
        LOG_ERR("创建动画图像失败");
        return -ENOMEM;
    }
    
    lv_obj_center(widget->obj);
    
    // 初始化为空闲状态
    struct bongo_cat_wpm_status_state init_state = { .wpm = 0 };
    set_animation(widget->obj, init_state);
    
    sys_slist_append(&widgets, &widget->node);
    widget_bongo_cat_init();
    
    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) {
    return widget ? widget->obj : NULL;
}

void zmk_widget_bongo_cat_set_wpm(struct zmk_widget_bongo_cat *widget,
                                  uint8_t wpm) {
    if (!widget || !widget->obj || !lv_obj_is_valid(widget->obj)) {
        LOG_WRN("无效的 widget 或对象");
        return;
    }
    
    struct bongo_cat_wpm_status_state state = {.wpm = wpm};
    set_animation(widget->obj, state);
}

/* ================= 销毁函数 ================= */

void zmk_widget_bongo_cat_destroy(struct zmk_widget_bongo_cat *widget) {
    if (!widget) return;
    
    // 从全局链表中移除
    struct zmk_widget_bongo_cat *prev = NULL;
    struct zmk_widget_bongo_cat *curr;
    
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, curr, node) {
        if (curr == widget) {
            if (prev) {
                sys_slist_remove(&widgets, &prev->node, &widget->node);
            } else {
                // 如果是第一个节点
                sys_slist_t *head = &widgets;
                sys_slist_remove(head, NULL, &widget->node);
            }
            break;
        }
        prev = curr;
    }
    
    // 停止并销毁动画对象
    if (widget->obj && lv_obj_is_valid(widget->obj)) {
        // 不需要调用 lv_animimg_stop，直接删除对象即可
        stop_fast_color_anim(widget->obj);
        lv_obj_del(widget->obj);
        widget->obj = NULL;
    }
}

/* ================= 清理函数 ================= */

void zmk_widget_bongo_cat_cleanup(void) {
    struct zmk_widget_bongo_cat *widget, *tmp;
    
    SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&widgets, widget, tmp, node) {
        zmk_widget_bongo_cat_destroy(widget);
    }
}