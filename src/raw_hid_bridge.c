#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <string.h>
#include <stdint.h>
#include <raw_hid/events.h>

#include "raw_hid_bridge.h"

LOG_MODULE_REGISTER(raw_hid_bridge, LOG_LEVEL_DBG);

ZMK_EVENT_IMPL(connected_notification);
ZMK_EVENT_IMPL(time_notification);
ZMK_EVENT_IMPL(volume_notification);
ZMK_EVENT_IMPL(os_notification);

typedef enum {
    HID_TIME = 0xAA, // random value, must match companion app
    HID_VOLUME,
    HID_LAYER,
    HID_OS,
} hid_data_type;

static bool hid_connected = false;


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

static uint8_t last_hid_volume = 0;
static uint8_t last_raised_volume = 0;

static void on_volume_timer(struct k_timer *dummy) {
    // prevent raising event with the same value multiple times
    if (last_raised_volume != last_hid_volume) {
        last_raised_volume = last_hid_volume;
        LOG_INF("raise_volume_notification %i", last_hid_volume);
        raise_volume_notification((struct volume_notification){.value = last_hid_volume});
    }
}

K_TIMER_DEFINE(volume_timer, on_volume_timer, NULL);

static void process_raw_hid_data(uint8_t *data) {
    LOG_INF("display_process_raw_hid_data - received data_type %u", data[0]);

    uint8_t data_type = data[0];
    switch (data_type) {
        case HID_TIME:
            raise_time_notification((struct time_notification){.hour = data[1], .minute = data[2]});
            break;
        case HID_VOLUME:
            last_hid_volume = data[1];

            // debounce volume change events
            if (k_timer_status_get(&volume_timer) > 0 || k_timer_remaining_get(&volume_timer) == 0) {
                k_timer_start(&volume_timer, K_MSEC(150), K_NO_WAIT);
                on_volume_timer(&volume_timer);
            }

            break;
        case HID_OS:
            LOG_INF("HID_OS data received: data[0]=%u, data[1]=%u, data[2]=%u", data[0], data[1], data[2]);
            raise_os_notification((struct os_notification){.system_type = data[1]});
            break;

    }
}

/* HID 接收事件回调 */
static int raw_hid_received_event_listener(const zmk_event_t *eh) {
    const struct raw_hid_received_event *event = as_raw_hid_received_event(eh);
    if (event) {
        process_raw_hid_data(event->data);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

/* 注册事件监听器 */
ZMK_LISTENER(raw_hid_listener, raw_hid_received_event_listener);
ZMK_SUBSCRIPTION(raw_hid_listener, raw_hid_received_event);
