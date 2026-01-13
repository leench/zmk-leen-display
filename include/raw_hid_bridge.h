#pragma once

#include <zmk/event_manager.h>

/**
 * @brief 获取最新接收到的 Raw HID 数据
 * 
 * @return const uint8_t* 指向 CONFIG_RAW_HID_REPORT_SIZE 长度的数据
 */
const uint8_t *raw_hid_bridge_get_buffer(void);

struct connected_notification {
    bool value;
};

ZMK_EVENT_DECLARE(connected_notification);

struct time_notification {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

ZMK_EVENT_DECLARE(time_notification);

struct volume_notification {
    uint8_t value;
};

ZMK_EVENT_DECLARE(volume_notification);

struct os_notification {
    uint8_t system_type;
};

ZMK_EVENT_DECLARE(os_notification);