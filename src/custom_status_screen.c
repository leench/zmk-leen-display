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

LOG_MODULE_REGISTER(custom_status_screen, LOG_LEVEL_DBG);

#define HID_UPDATE_PERIOD_MS 50  // 每50ms更新一次

// 同步阈值（秒），当 HID 时间与本地时间差 <= 该值时不进行同步
#define CLOCK_SYNC_THRESHOLD_S 2

static struct k_timer hid_timer;
static struct k_work hid_work;

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
 *      HID 工作函数
 * ============================ */
static void hid_work_handler(struct k_work *work) {
    const uint8_t *buf = raw_hid_bridge_get_buffer();

    // 没有新 HID 数据
    if (!buf) {
        //LOG_DBG("No HID data");
        return;
    }
    LOG_DBG("Processing HID cmd=%d", buf[0]);

    switch (buf[0]) {
        case 170:  // 时间命令：buf[1]=hour, buf[2]=min, buf[3]=sec
            if (CONFIG_RAW_HID_REPORT_SIZE >= 4) {
                zmk_widget_clock_sync(&clock_widget, buf[1], buf[2], buf[3], CLOCK_SYNC_THRESHOLD_S);
            }
            break;

        case 171:  // 音量命令：buf[1]=volume
            if (CONFIG_RAW_HID_REPORT_SIZE >= 2) {
                zmk_widget_volume_set(&volume_widget, buf[1]);
            }
            break;

        case 173:  // 系统类型命令：buf[1]=系统类型 (1=Windows, 2=Linux, 3=macOS)
            if (CONFIG_RAW_HID_REPORT_SIZE >= 2) {
                // 保存系统类型到全局变量，供其他模块使用
                system_type = buf[1];
                LOG_DBG("System type updated: %d", system_type);
                
                // 更新系统图标显示（转换为枚举类型）
                zmk_widget_sysicon_set_system(&sysicon_widget, (enum system_type)system_type);

                // 更新修饰键widget的系统类型
                zmk_widget_modifiers_set_system_type((enum system_type)system_type);
            }
            break;

        default:
            // 预留用于扩展
            break;
    }
}

/* ============================
 *     定时器回调
 * ============================ */
static void hid_timer_handler(struct k_timer *timer_id) {
    k_work_submit(&hid_work);
}

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
    lv_obj_align(zmk_widget_connection_obj(&connection_widget), LV_ALIGN_TOP_LEFT, 15, 12);


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

    /* ---- 启动 HID 处理定时器 ---- */
    k_work_init(&hid_work, hid_work_handler);
    k_timer_init(&hid_timer, hid_timer_handler, NULL);
    k_timer_start(&hid_timer, K_MSEC(HID_UPDATE_PERIOD_MS), K_MSEC(HID_UPDATE_PERIOD_MS));

    return screen;
}