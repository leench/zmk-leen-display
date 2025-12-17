#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(bongo_cat_widget, LOG_LEVEL_DBG);

#include <zmk/display.h>
#include <lvgl.h>

#include "bongo_cat.h"

// 动画帧
LV_IMG_DECLARE(bongo_cat_left2);
LV_IMG_DECLARE(bongo_cat_right2);

static const lv_img_dsc_t *anim_frames[] = {
    &bongo_cat_right2,
    &bongo_cat_left2,
};
#define ANIM_FRAME_COUNT (sizeof(anim_frames) / sizeof(anim_frames[0]))
#define ANIM_SPEED 500 // ms

// widget 列表
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

// 当前帧索引
struct anim_data {
    lv_obj_t *img;
    uint8_t frame;
};

// 定时器回调
static void anim_timer_cb(lv_timer_t *timer) {
    struct anim_data *data = (struct anim_data *)timer->user_data;
    data->frame = (data->frame + 1) % ANIM_FRAME_COUNT;
    lv_img_set_src(data->img, anim_frames[data->frame]);
}

// 初始化动画
static void start_animation(lv_obj_t *img_obj) {
    static struct anim_data data;
    data.img = img_obj;
    data.frame = 0;

    lv_timer_t *timer = lv_timer_create(anim_timer_cb, ANIM_SPEED, &data);
    lv_timer_ready(timer);
}

// 初始化 Bongo Cat Widget
int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    if (!widget || !parent) {
        LOG_ERR("初始化参数无效");
        return -EINVAL;
    }

    widget->obj = lv_img_create(parent);
    if (!widget->obj) return -ENOMEM;

    lv_img_set_src(widget->obj, anim_frames[0]);
    lv_obj_center(widget->obj);

    start_animation(widget->obj);
    sys_slist_append(&widgets, &widget->node);

    LOG_INF("Bongo Cat Widget 初始化完成（循环动画）");
    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) {
    return widget ? widget->obj : NULL;
}
