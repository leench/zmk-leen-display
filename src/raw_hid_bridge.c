#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <string.h>
#include <stdint.h>

#include "raw_hid_bridge.h"
#include <raw_hid/events.h>

LOG_MODULE_REGISTER(raw_hid_bridge, LOG_LEVEL_DBG);

/* 静态 HID 缓冲区 */
static uint8_t hid_buf[CONFIG_RAW_HID_REPORT_SIZE];
static atomic_t hid_buf_ready = ATOMIC_INIT(false);

/* 对外接口：获取一次 HID 数据 */
const uint8_t *raw_hid_bridge_get_buffer(void)
{
    atomic_val_t prev = atomic_cas(&hid_buf_ready, true, false);
    if (prev != (atomic_val_t)true) {
        return NULL;
    }
    return hid_buf;
}

/* HID 接收事件回调 */
static int raw_hid_received_event_listener(const zmk_event_t *eh) {
    const struct raw_hid_received_event *event = as_raw_hid_received_event(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    /* 固定长度复制 */
    memcpy(hid_buf, event->data, CONFIG_RAW_HID_REPORT_SIZE);

    /* 标记为新数据 */
    atomic_set(&hid_buf_ready, true);

    LOG_DBG("HID 报告已接收, buf[0]=%u", (unsigned)hid_buf[0]);
    return ZMK_EV_EVENT_BUBBLE;
}

/* 注册事件监听器 */
ZMK_LISTENER(raw_hid_listener, raw_hid_received_event_listener);
ZMK_SUBSCRIPTION(raw_hid_listener, raw_hid_received_event);
