#ifndef ZMK_WIDGET_BONGO_CAT_H
#define ZMK_WIDGET_BONGO_CAT_H

#include <zephyr/kernel.h>
#include <lvgl.h>

// 动画状态枚举
enum anim_state {
    anim_state_none,
    anim_state_idle,
    anim_state_slow,
    anim_state_mid,
    anim_state_fast
};

struct zmk_widget_bongo_cat {
    lv_obj_t *obj;
    sys_snode_t node;
};

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget);
void zmk_widget_bongo_cat_set_wpm(struct zmk_widget_bongo_cat *widget, uint8_t wpm);

#endif /* ZMK_WIDGET_BONGO_CAT_H */