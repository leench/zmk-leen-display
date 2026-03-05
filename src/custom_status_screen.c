#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zephyr/logging/log.h>

#include "raw_hid_bridge.h"

#if IS_ENABLED(CONFIG_ZMK_WIDGET_CLOCK)
#include "widgets/clock.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_VOLUME)
#include "widgets/volume.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY)
#include "widgets/battery.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER)
#include "widgets/layer.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_CONNECTION)
#include "widgets/connection.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BONGO_CAT)
#include "widgets/bongo_cat.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_SYSICON)
#include "widgets/sysicon.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_MODIFIERS)
#include "widgets/modifiers.h"
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
#include "widgets/wpm_status.h"
#endif

LOG_MODULE_REGISTER(custom_status_screen, LOG_LEVEL_INF);

#define HID_UPDATE_PERIOD_MS 50  // 每50ms更新一次

// static struct k_timer hid_timer;
// static struct k_work hid_work;

// Widgets
#if IS_ENABLED(CONFIG_ZMK_WIDGET_CLOCK)
static struct zmk_widget_clock clock_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_VOLUME)
static struct zmk_widget_volume volume_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY)
static struct zmk_widget_battery battery_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER)
static struct zmk_widget_layer layer_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_CONNECTION)
static struct zmk_widget_connection connection_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BONGO_CAT)
static struct zmk_widget_bongo_cat bongo_cat_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_SYSICON)
static struct zmk_widget_sysicon sysicon_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_MODIFIERS)
static struct zmk_widget_modifiers modifiers_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

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


#if IS_ENABLED(CONFIG_ZMK_WIDGET_CONNECTION)
    /* ---- 初始化 Connection Widget ---- */
    zmk_widget_connection_init(&connection_widget, screen);
    lv_obj_align(zmk_widget_connection_obj(&connection_widget), LV_ALIGN_TOP_LEFT, 8, 12);
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_BONGO_CAT)
    /* ---- 初始化 bonge cat Widget ---- */
    zmk_widget_bongo_cat_init(&bongo_cat_widget, screen);
    lv_obj_align(zmk_widget_bongo_cat_obj(&bongo_cat_widget), LV_ALIGN_TOP_RIGHT, -15, 12);
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_SYSICON)
    /* ---- 初始化 system icon Widget ---- */
#if IS_ENABLED(CONFIG_ZMK_WIDGET_BONGO_CAT)
    zmk_widget_sysicon_init(&sysicon_widget, screen, &bongo_cat_widget);
#else
    zmk_widget_sysicon_init(&sysicon_widget, screen, NULL);
#endif
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_MODIFIERS)
    /* ---- 初始化 modifiers Widget ---- */
    zmk_widget_modifiers_init(&modifiers_widget, screen);
    lv_obj_align(zmk_widget_modifiers_obj(&modifiers_widget), LV_ALIGN_TOP_MID, 0, 10);
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_CLOCK)
    /* ---- 初始化 Clock Widget ---- */
    zmk_widget_clock_init(&clock_widget, screen);
    lv_obj_align(zmk_widget_clock_obj(&clock_widget), LV_ALIGN_CENTER, 5, -5);
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER)
    /* ---- 初始化 Layer Widget ---- */
#if IS_ENABLED(CONFIG_ZMK_WIDGET_CLOCK)
    zmk_widget_layer_init(&layer_widget, screen, &clock_widget);
#else
    zmk_widget_layer_init(&layer_widget, screen, NULL);
#endif
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_VOLUME)
    /* ---- 初始化 Volume Widget ---- */
#if IS_ENABLED(CONFIG_ZMK_WIDGET_CLOCK)
    zmk_widget_volume_init(&volume_widget, screen, &clock_widget);
#else
    zmk_widget_volume_init(&volume_widget, screen, NULL);
#endif
#endif


#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY)
    /* ---- 初始化电池条 ---- */
    // 初始化电池部件
    zmk_widget_battery_bar_init(&battery_widget, screen);
    // 设置电池部件高度
    lv_obj_set_height(zmk_widget_battery_bar_obj(&battery_widget), 40);
    // 对齐到屏幕底部
    lv_obj_align(zmk_widget_battery_bar_obj(&battery_widget), LV_ALIGN_BOTTOM_MID, 0, -8);
#endif
    

    LOG_INF("屏幕和 widgets 已创建");

    return screen;
}