/*
 * CST816D 长按进入bootloader功能
 * 针对 nice!nano v2 (nRF52840)
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zmk/keymap.h>
#include <hal/nrf_power.h>
#include "input_cst816d.h"

LOG_MODULE_REGISTER(touch_func, LOG_LEVEL_INF);

/* 工作项 */
static struct k_work_delayable work;

/* 进入 bootloader 的函数 */
static void enter_bootloader(void)
{
    LOG_INF("Entering bootloader mode...");
    
    nrf_power_gpregret_set(NRF_POWER, 0, 0x57);
    sys_reboot(SYS_REBOOT_COLD);
}

/* 工作处理函数 */
static void work_handler(struct k_work *work)
{
    struct cst816d_touch touch;

    if (cst816d_read_touch(&touch) && touch.gesture == CST816D_GESTURE_LONG_PRESS) {
        LOG_INF("Long press detected - Entering bootloader");
        uint8_t current_layer = zmk_keymap_highest_layer_active();
        LOG_INF("Current layer: %d", current_layer);

        if (current_layer == 6) {
            LOG_INF("Current layer: 6");
        }

        k_sleep(K_MSEC(1000));
        enter_bootloader();
    }

    k_work_schedule((struct k_work_delayable *)work, K_MSEC(100));
}

/* 初始化 */
static int init(void)
{
    k_work_init_delayable(&work, work_handler);
    k_work_schedule(&work, K_MSEC(100));
    LOG_INF("Touch initialized: Long press -> bootloader");
    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);