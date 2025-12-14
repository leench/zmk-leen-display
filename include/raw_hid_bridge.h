#pragma once
#include <stdint.h>

/**
 * @brief 获取最新接收到的 Raw HID 数据
 * 
 * @return const uint8_t* 指向 CONFIG_RAW_HID_REPORT_SIZE 长度的数据
 */
const uint8_t *raw_hid_bridge_get_buffer(void);
