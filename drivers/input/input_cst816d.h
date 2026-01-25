/* drivers/input/input_cst816d.h */
#ifndef INPUT_CST816D_H
#define INPUT_CST816D_H

#include <zephyr/device.h>
#include <stdbool.h>
#include <stdint.h>

/* I2C 地址 */
#define CST816D_ADDR 0x15

/* 轮询间隔(ms) */
#define CST816D_POLL_MS 100

/* 手势定义 */
enum cst816d_gesture {
    CST816D_GESTURE_NONE = 0x00,         /* 无手势 (0) */
    CST816D_GESTURE_SLIDE_DOWN = 0x01,   /* 向下滑动 (1) */
    CST816D_GESTURE_SLIDE_UP = 0x02,     /* 向上滑动 (2) */
    CST816D_GESTURE_SLIDE_LEFT = 0x03,   /* 向左滑动 (3) */
    CST816D_GESTURE_SLIDE_RIGHT = 0x04,  /* 向右滑动 (4) */
    CST816D_GESTURE_SINGLE_TAP = 0x05,   /* 单击/轻触 (5) */
    CST816D_GESTURE_DOUBLE_TAP = 0x0B,   /* 双击 (11) */
    CST816D_GESTURE_LONG_PRESS = 0x0C    /* 长按 (12) */
};

/* 触摸模式枚举 */
enum cst816d_touch_mode {
    CST816D_MODE_GESTURE = 0,    // 手势模式
    CST816D_MODE_POINT = 1,      // 点模式（仅坐标）
    CST816D_MODE_MIXED = 2,      // 混合模式
};

/* 触摸数据结构 */
struct cst816d_touch {
    bool touched;                /* 是否有触摸 */
    uint16_t x;                  /* X坐标 */
    uint16_t y;                  /* Y坐标 */
    enum cst816d_gesture gesture; /* 检测到的手势 */
};

/* 寄存器地址定义 */
#define CST816D_REG_CHIP_ID     0xA7
#define CST816D_REG_GESTURE     0x01
#define CST816D_REG_FINGER      0x02
#define CST816D_REG_XPOS_H      0x03
#define CST816D_REG_XPOS_L      0x04
#define CST816D_REG_YPOS_H      0x05
#define CST816D_REG_YPOS_L      0x06
#define CST816D_REG_AUTO_SLEEP  0xFE

/* 模式设置相关寄存器 */
#define CST816D_REG_IRQ_CTL     0xFA    // 中断控制
#define CST816D_REG_NOR_SCAN_PER 0xFB   // 正常扫描周期
#define CST816D_REG_IRQ_PULSE_WIDTH 0xFC // 中断脉冲宽度
#define CST816D_REG_MOTION_MASK 0xFD    // 运动掩码

/* 模式设置相关值 */
#define CST816D_IRQ_CTL_GESTURE 0x11    // 手势模式
#define CST816D_IRQ_CTL_POINT   0x41    // 点模式
#define CST816D_IRQ_CTL_MIXED   0x71    // 混合模式

#define CST816D_NOR_SCAN_PER_DEFAULT 0x01  // 默认扫描周期 10ms
#define CST816D_IRQ_PULSE_WIDTH_DEFAULT 0x0F // 中断脉冲宽度 1.5ms

#define CST816D_MOTION_MASK_DCLICK 0x01  // 双击使能

/* 驱动 API - 与现有代码兼容 */
bool cst816d_init(const struct device *i2c_dev);
bool cst816d_read_touch(struct cst816d_touch *touch);
bool cst816d_set_mode(enum cst816d_touch_mode mode);

#endif /* INPUT_CST816D_H */