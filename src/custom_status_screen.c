#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zephyr/logging/log.h>

#include "raw_hid_bridge.h"
#include "widgets/clock.h"
#include "widgets/volume.h"
#include "widgets/battery.h"
#include "widgets/layer.h"
#include "widgets/connection.h"
#include "widgets/bongo_cat.h"
#include "widgets/sysicon.h"
#include "widgets/modifiers.h"

LOG_MODULE_REGISTER(custom_status_screen, LOG_LEVEL_INF);

#define HID_UPDATE_PERIOD_MS 50  // 每50ms更新一次

// static struct k_timer hid_timer;
// static struct k_work hid_work;

// Widgets
static struct zmk_widget_clock clock_widget;
static struct zmk_widget_volume volume_widget;
static struct zmk_widget_battery battery_widget;
static struct zmk_widget_layer layer_widget;
static struct zmk_widget_connection connection_widget;
static struct zmk_widget_bongo_cat bongo_cat_widget;
static struct zmk_widget_sysicon sysicon_widget;
static struct zmk_widget_modifiers modifiers_widget;

uint8_t system_type = 0;  // 定义并初始化为0（未设置） sysicon.h


/* ============================
 *          屏幕创建
 * ============================ */
lv_obj_t *zmk_display_status_screen(void) {
    LOG_INF("创建显示屏幕...");

    lv_obj_t *screen = lv_obj_create(NULL);
    lv_scr_load(screen);

    // 黑色背景
    static lv_style_t screen_style;
    lv_style_init(&screen_style);
    lv_style_set_bg_color(&screen_style, lv_color_black());
    lv_style_set_bg_opa(&screen_style, LV_OPA_COVER);
    lv_obj_add_style(screen, &screen_style, 0);


    /* ---- 初始化 Connection Widget ---- */
    zmk_widget_connection_init(&connection_widget, screen);
    lv_obj_align(zmk_widget_connection_obj(&connection_widget), LV_ALIGN_TOP_LEFT, 8, 12);


    /* ---- 初始化 bonge cat Widget ---- */
    zmk_widget_bongo_cat_init(&bongo_cat_widget, screen);
    lv_obj_align(zmk_widget_bongo_cat_obj(&bongo_cat_widget), LV_ALIGN_TOP_RIGHT, -15, 12);


    /* ---- 初始化 system icon Widget ---- */
    zmk_widget_sysicon_init(&sysicon_widget, screen, &bongo_cat_widget);


    /* ---- 初始化 modifiers Widget ---- */
    zmk_widget_modifiers_init(&modifiers_widget, screen);
    lv_obj_align(zmk_widget_modifiers_obj(&modifiers_widget), LV_ALIGN_TOP_MID, 0, 10);


    /* ---- 初始化 Clock Widget ---- */
    zmk_widget_clock_init(&clock_widget, screen);
    lv_obj_align(zmk_widget_clock_obj(&clock_widget), LV_ALIGN_CENTER, 5, -5);


    /* ---- 初始化 Layer Widget ---- */
    zmk_widget_layer_init(&layer_widget, screen, &clock_widget);


    /* ---- 初始化 Volume Widget ---- */
    zmk_widget_volume_init(&volume_widget, screen, &clock_widget);


    /* ---- 初始化电池条 ---- */
    // 初始化电池部件
    zmk_widget_battery_bar_init(&battery_widget, screen);
    // 设置电池部件高度
    lv_obj_set_height(zmk_widget_battery_bar_obj(&battery_widget), 40);
    // 对齐到屏幕底部
    lv_obj_align(zmk_widget_battery_bar_obj(&battery_widget), LV_ALIGN_BOTTOM_MID, 0, -8);
    

    LOG_INF("屏幕和 widgets 已创建");

    return screen;
}