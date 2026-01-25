/*
 * CST816D 电容触摸屏驱动
 * 
 * 概述
 * =====
 * CST816D 是一款电容式触摸屏控制器，支持单点触摸和多种手势识别。
 * 本驱动为 Zephyr RTOS 编写，支持设备树配置。
 * 
 * 特性
 * =====
 * - 单点触摸检测
 * - 手势识别（滑动、单击、双击、长按）
 * - 设备树配置
 * - I2C 接口
 * - 硬件复位支持
 * - 中断支持（可选）
 * 
 * 设备树配置
 * ==========
 * 在设备树 overlay 文件中添加：
 * 
 * &i2c0 {
 *     status = "okay";
 *     clock-frequency = <100000>;
 * 
 *     cst816d: cst816d@15 {
 *         compatible = "hynitron,cst816d";
 *         reg = <0x15>;                    // I2C 7位地址
 *         irq-gpios = <&gpio0 8 GPIO_ACTIVE_LOW>;   // 中断引脚（可选）
 *         rst-gpios = <&gpio0 31 GPIO_ACTIVE_LOW>;  // 复位引脚（可选）
 *         status = "okay";
 *     };
 * };
 * 
 * 引脚说明：
 * - irq-gpios: 中断引脚，低电平有效（可选）
 * - rst-gpios: 复位引脚，低电平有效（可选）
 * 
 * 寄存器映射
 * ==========
 * 地址  名称           描述
 * ----  ------------- ----------------------------
 * 0xA7  ChipID        芯片ID（默认0xB4）
 * 0x01  Gesture       手势识别
 * 0x02  Finger        触摸点数量
 * 0x03  XposH         X坐标高4位
 * 0x04  XposL         X坐标低8位
 * 0x05  YposH         Y坐标高4位
 * 0x06  YposL         Y坐标低8位
 * 0xFE  DisAutoSleep  禁止自动睡眠
 * 
 * 手势定义
 * ========
 * 手势码  宏定义                       描述
 * ------  --------------------------  ----------
 * 0x00    CST816D_GESTURE_NONE        无手势
 * 0x01    CST816D_GESTURE_SLIDE_DOWN  向下滑动
 * 0x02    CST816D_GESTURE_SLIDE_UP    向上滑动
 * 0x03    CST816D_GESTURE_SLIDE_LEFT  向左滑动
 * 0x04    CST816D_GESTURE_SLIDE_RIGHT 向右滑动
 * 0x05    CST816D_GESTURE_SINGLE_TAP  单击
 * 0x0B    CST816D_GESTURE_DOUBLE_TAP  双击
 * 0x0C    CST816D_GESTURE_LONG_PRESS  长按
 * 
 * API 使用
 * ========
 * 1. 包含头文件：
 *    #include "input_cst816d.h"
 * 
 * 2. 数据结构：
 *    struct cst816d_touch {
 *        bool touched;                // 是否有触摸
 *        uint16_t x;                  // X坐标 (0-240)
 *        uint16_t y;                  // Y坐标 (0-240)
 *        enum cst816d_gesture gesture; // 检测到的手势
 *    };
 * 
 * 3. 函数接口：
 *    // 初始化驱动（兼容性API，驱动通常通过设备树自动初始化）
 *    bool cst816d_init(const struct device *i2c_dev);
 *    
 *    // 读取触摸数据
 *    bool cst816d_read_touch(struct cst816d_touch *touch);
 * 
 * 4. 使用示例：
 *    #include "input_cst816d.h"
 *    
 *    void check_touch(void) {
 *        struct cst816d_touch touch;
 *        
 *        if (cst816d_read_touch(&touch)) {
 *            if (touch.touched) {
 *                printk("Touch at (%d, %d)\n", touch.x, touch.y);
 *            }
 *            
 *            switch (touch.gesture) {
 *            case CST816D_GESTURE_SINGLE_TAP:
 *                printk("Single tap\n");
 *                break;
 *            case CST816D_GESTURE_DOUBLE_TAP:
 *                printk("Double tap\n");
 *                break;
 *            case CST816D_GESTURE_LONG_PRESS:
 *                printk("Long press\n");
 *                break;
 *            }
 *        }
 *    }
 * 
 * 调试信息
 * ========
 * 驱动启用日志后，会输出以下信息：
 * - 设备树配置详情
 * - 硬件复位过程
 * - 芯片ID读取结果
 * - 触摸坐标和手势
 * 
 * 常见问题
 * ========
 * 1. 芯片无响应
 *    - 检查I2C地址是否为0x15（7位地址）
 *    - 检查SCL/SDA上拉电阻（4.7kΩ到3.3V）
 *    - 检查RST引脚电平（正常应为高电平3.3V）
 *    - 检查电源电压（3.3V）
 * 
 * 2. 触摸坐标错误
 *    - 确认屏幕分辨率配置
 *    - 检查触摸屏校准
 * 
 * 3. 手势不识别
 *    - 确认手势使能寄存器配置
 *    - 检查手势检测参数
 * 
 * 版本历史
 * ========
 * v1.0.0 - 初始版本
 *         - 基本触摸功能
 *         - 手势识别
 *         - 设备树支持
 * 
 * 注意事项
 * ========
 * 1. I2C总线必须配置上拉电阻（SCL/SDA各4.7kΩ到3.3V）
 * 2. RST和INT引脚在设备树中配置为GPIO_ACTIVE_LOW
 * 3. 芯片复位后需要至少100ms稳定时间
 * 4. 建议I2C时钟频率不超过100kHz
 * 
 * 作者
 * ====
 * [你的名字/团队]
 * 
 * 许可证
 * ======
 * SPDX-License-Identifier: Apache-2.0
 */

 
#define DT_DRV_COMPAT hynitron_cst816d

#include "input_cst816d.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(cst816d, LOG_LEVEL_INF);

/* 调试配置 - 方便修改默认模式 */
#define CST816D_DEBUG_DEFAULT_MODE CST816D_MODE_POINT  // 修改这里即可
// 可选值：
// #define CST816D_DEBUG_DEFAULT_MODE CST816D_MODE_GESTURE  // 手势模式
// #define CST816D_DEBUG_DEFAULT_MODE CST816D_MODE_POINT    // 点模式  
// #define CST816D_DEBUG_DEFAULT_MODE CST816D_MODE_MIXED    // 混合模式

//#define CONFIG_INPUT_CST816D_INTERRUPT y

/* 配置结构 - 从设备树读取 */
struct cst816d_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec rst_gpio;
    struct gpio_dt_spec int_gpio;
    uint16_t rotation;      /* 旋转角度: 0, 90, 180, 270 */
    bool swap_xy;          /* 是否交换XY坐标 */
    bool invert_x;         /* 是否反转X坐标 */
    bool invert_y;         /* 是否反转Y坐标 */
    uint16_t width;        /* 屏幕宽度（像素） */
    uint16_t height;       /* 屏幕高度（像素） */
};

/* 放到驱动的 config/data 结构里 */
struct pointing_cfg {
    uint16_t axis_x;
    uint16_t axis_y;
};

struct pointing_data {
    bool prev_valid;
    int32_t prev_x;
    int32_t prev_y;
    /* 固定点累积器：累积 (dx * NUM)，除以 DEN 得到上报值，保留余数避免丢失小移动 */
    int32_t acc_x;
    int32_t acc_y;
    /* 可配置灵敏度：rel = (dx * SENSITIVITY_NUM) / SENSITIVITY_DEN */
    int32_t sensitivity_num;
    int32_t sensitivity_den;
};

/* 设备数据 */
struct cst816d_data {
    struct k_work_delayable poll_work;
    struct cst816d_touch last_touch;
    const struct device *dev;
    bool initialized;
    enum cst816d_touch_mode current_mode;
#ifdef CONFIG_INPUT_CST816D_INTERRUPT
    struct k_work work;
	struct gpio_callback int_gpio_cb;
#endif
    const struct device *input_dev;
    struct pointing_data pointing_data;
};

/* 全局设备指针 - 供API使用 */
static struct cst816d_data *g_cst816d_data = NULL;

/* 坐标转换函数 */
static void cst816d_transform_coordinates(const struct cst816d_config *cfg, 
                                        uint16_t *x, uint16_t *y)
{
    uint16_t orig_x = *x;
    uint16_t orig_y = *y;
    uint16_t temp;
    
    /* 从配置获取屏幕分辨率 */
    const uint16_t max_x = cfg->width > 0 ? cfg->width - 1 : 239;
    const uint16_t max_y = cfg->height > 0 ? cfg->height - 1 : 239;
    
    /* 第一步：应用旋转 */
    switch ((uint16_t)cfg->rotation) {
        case 90:
            /* (x, y) -> (y, max_x - x) */
            temp = *x;
            *x = *y;
            *y = max_x - temp;
            break;
            
        case 180:
            /* (x, y) -> (max_x - x, max_y - y) */
            *x = max_x - *x;
            *y = max_y - *y;
            break;
            
        case 270:
            /* (x, y) -> (max_y - y, x) */
            temp = *x;
            *x = max_y - *y;
            *y = temp;
            break;
            
        case 0:
        default:
            /* 不旋转 */
            break;
    }
    
    /* 第二步：应用交换XY（在旋转之后） */
    if (cfg->swap_xy) {
        temp = *x;
        *x = *y;
        *y = temp;
    }
    
    /* 第三步：应用反转（在交换之后） */
    if (cfg->invert_x) {
        *x = max_x - *x;
    }
    
    if (cfg->invert_y) {
        *y = max_y - *y;
    }
    
    /* 确保坐标在有效范围内 */
    if (*x > max_x) *x = max_x;
    if (*y > max_y) *y = max_y;
    
    /* 调试日志 */
    if (orig_x != *x || orig_y != *y) {
        LOG_DBG("Coord transform: (%d,%d) -> (%d,%d) [rot=%d°, swap=%s, inv_x=%s, inv_y=%s, res=%dx%d]",
                orig_x, orig_y, *x, *y, cfg->rotation, 
                cfg->swap_xy ? "yes" : "no",
                cfg->invert_x ? "yes" : "no",
                cfg->invert_y ? "yes" : "no",
                cfg->width, cfg->height);
    }
}

/* 初始化 data（例如在 probe/init 中） */
static void pointing_init_data(struct pointing_data *d)
{
    d->prev_valid = false;
    d->prev_x = 0;
    d->prev_y = 0;
    d->acc_x = 0;
    d->acc_y = 0;
    /* 默认放大倍数 8（根据需要调整）*/
    d->sensitivity_num = 4;
    d->sensitivity_den = 1;
}

/*
 * 将绝对坐标转换为相对并上报。
 * abs_x/abs_y: 设备上报的绝对坐标（0..239）
 * touching: 当前是否有触摸（按下）
 */
static void cst816d_handle_abs_event(const struct device *dev,
                                 int32_t abs_x, int32_t abs_y,
                                 bool touching)
{
    struct cst816d_data *cst_data = dev->data;
    const struct cst816d_config *cfg = dev->config;
    struct pointing_data *data = &cst_data->pointing_data;
    
    static bool first_touch_reported = false;
    
    /* 从配置获取屏幕分辨率 */
    const int32_t max_x = cfg->width > 0 ? cfg->width - 1 : 239;
    const int32_t max_y = cfg->height > 0 ? cfg->height - 1 : 239;
    
    /* 非触摸状态：重置 */
    if (!touching) {
        data->prev_valid = false;
        data->acc_x = 0;
        data->acc_y = 0;
        first_touch_reported = false;
        return;
    }

    /* 限定范围 */
    if (abs_x < 0) abs_x = 0;
    if (abs_x > max_x) abs_x = max_x;
    if (abs_y < 0) abs_y = 0;
    if (abs_y > max_y) abs_y = max_y;

    /* 第一次触摸：只记录基线，不产生移动 */
    if (!data->prev_valid) {
        data->prev_x = abs_x;
        data->prev_y = abs_y;
        data->prev_valid = true;
        first_touch_reported = false;
        LOG_INF("First touch at (%d, %d)", abs_x, abs_y);
        return;
    }

    /* 计算移动 */
    int32_t dx = abs_x - data->prev_x;
    int32_t dy = abs_y - data->prev_y;

    /* 防跳变：第一次有效移动前忽略 */
    if (!first_touch_reported) {
        if (abs(dx) > 2 || abs(dy) > 2) {  // 降低阈值到2
            first_touch_reported = true;
        } else {
            /* 微小移动，更新基线但不报告 */
            data->prev_x = abs_x;
            data->prev_y = abs_y;
            return;
        }
    }

    /* 防突变 */
    if (abs(dx) > 20 || abs(dy) > 20) {  // 降低阈值到20
        LOG_INF("Jump reset: dx=%d, dy=%d", dx, dy);
        data->prev_x = abs_x;
        data->prev_y = abs_y;
        return;
    }

    /* 关键修改：微小移动也累积，不直接返回 */
    /* 正常处理所有移动，包括微小移动 */
    data->acc_x += dx * data->sensitivity_num;
    data->acc_y += dy * data->sensitivity_num;

    int32_t rel_x = 0, rel_y = 0;
    if (data->sensitivity_den != 0) {
        rel_x = data->acc_x / data->sensitivity_den;
        rel_y = data->acc_y / data->sensitivity_den;
        data->acc_x = data->acc_x % data->sensitivity_den;
        data->acc_y = data->acc_y % data->sensitivity_den;
    }

    /* 即使移动很小，只要累积到足够值就上报 */
    if (rel_x != 0 || rel_y != 0) {
        input_report_rel(dev, INPUT_REL_X, rel_x, false, K_FOREVER);
        input_report_rel(dev, INPUT_REL_Y, rel_y, true, K_FOREVER);
        LOG_INF("Move: rel(%d,%d) from (%d,%d) acc(%d,%d)", 
                rel_x, rel_y, dx, dy, data->acc_x, data->acc_y);
    } else {
        /* 累积但未达到上报阈值，记录日志 */
        LOG_INF("Accumulating: dx=%d, dy=%d, acc(%d,%d)", 
                dx, dy, data->acc_x, data->acc_y);
    }

    data->prev_x = abs_x;
    data->prev_y = abs_y;
}

/* 检查设备树配置并打印信息 */
static int cst816d_check_config(const struct device *dev)
{
    const struct cst816d_config *cfg = dev->config;
    int ret = 0;
    
    LOG_INF("=== CST816D Device Tree Configuration ===");
    
    /* 检查I2C配置 */
    LOG_INF("I2C Configuration:");
    LOG_INF("  Bus: %s", cfg->i2c.bus ? cfg->i2c.bus->name : "NULL");
    LOG_INF("  Address: 0x%02X", cfg->i2c.addr);
    
    /* 检查方向配置 */
    LOG_INF("Orientation Configuration:");
    LOG_INF("  Rotation: %d degrees", cfg->rotation);
    LOG_INF("  Swap XY: %s", cfg->swap_xy ? "yes" : "no");
    LOG_INF("  Invert X: %s", cfg->invert_x ? "yes" : "no");
    LOG_INF("  Invert Y: %s", cfg->invert_y ? "yes" : "no");
    
    /* 检查屏幕分辨率 */
    LOG_INF("Screen Resolution:");
    LOG_INF("  Width: %d pixels", cfg->width);
    LOG_INF("  Height: %d pixels", cfg->height);

    if (!device_is_ready(cfg->i2c.bus)) {
        LOG_ERR("I2C bus device not ready");
        ret = -ENODEV;
    } else {
        LOG_INF("I2C bus is ready");
    }
    
    /* 检查RST GPIO配置 */
    if (cfg->rst_gpio.port) {
        LOG_INF("RST GPIO: %s, pin=%d, flags=0x%x",
                cfg->rst_gpio.port->name, cfg->rst_gpio.pin, cfg->rst_gpio.dt_flags);
        
        if (!gpio_is_ready_dt(&cfg->rst_gpio)) {
            LOG_WRN("RST GPIO device not ready");
        }
    } else {
        LOG_INF("RST GPIO: Not configured");
    }
    
    /* 检查INT GPIO配置 */
    if (cfg->int_gpio.port) {
        LOG_INF("INT GPIO: %s, pin=%d, flags=0x%x",
                cfg->int_gpio.port->name, cfg->int_gpio.pin, cfg->int_gpio.dt_flags);
        
        if (!gpio_is_ready_dt(&cfg->int_gpio)) {
            LOG_WRN("INT GPIO device not ready");
        }
    } else {
        LOG_INF("INT GPIO: Not configured");
    }
    
    LOG_INF("=== Configuration Check Complete ===");
    return ret;
}

/* I2C读写函数 - 使用设备树配置 */
static int cst816d_i2c_read_internal(const struct device *dev, uint8_t reg, uint8_t *data, size_t len)
{
    const struct cst816d_config *cfg = dev->config;
    return i2c_write_read(cfg->i2c.bus, CST816D_ADDR, &reg, 1, data, len);
}

static int cst816d_i2c_write_internal(const struct device *dev, uint8_t reg, uint8_t data)
{
    const struct cst816d_config *cfg = dev->config;
    uint8_t buf[2] = {reg, data};
    return i2c_write(cfg->i2c.bus, buf, sizeof(buf), CST816D_ADDR);
}

/* 硬件复位 - 使用设备树GPIO配置 */
static void cst816d_hardware_reset(const struct device *dev)
{
    const struct cst816d_config *cfg = dev->config;

    LOG_INF("Performing hardware reset using DT GPIO...");
    
    /* 配置引脚为输出，考虑ACTIVE_LOW标志 */
    int ret = gpio_pin_configure_dt(&cfg->rst_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure RST GPIO: %d", ret);
        return;
    }
    
    /* 复位序列 - 正确处理ACTIVE_LOW */
    LOG_INF("RST GPIO: pin=%d, flags=0x%x", cfg->rst_gpio.pin, cfg->rst_gpio.dt_flags);
    
    /* 拉低复位（有效电平取决于ACTIVE_LOW） */
    gpio_pin_set_dt(&cfg->rst_gpio, 1);  /* ACTIVE_LOW: 1=低电平 */
    LOG_INF("RST set LOW (active)");
    k_msleep(10);
    
    /* 拉高释放复位（无效电平） */
    gpio_pin_set_dt(&cfg->rst_gpio, 0);  /* ACTIVE_LOW: 0=高电平 */
    LOG_INF("RST set HIGH (inactive)");
    k_msleep(50);
    
    LOG_INF("Hardware reset complete");
}

/* 配置INT引脚 - 使用设备树配置 */
static void cst816d_configure_int_pin(const struct device *dev)
{
    const struct cst816d_config *cfg = dev->config;
    
    /* 配置为输入，考虑ACTIVE_LOW标志 */
    int ret = gpio_pin_configure_dt(&cfg->int_gpio, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure INT GPIO: %d", ret);
        return;
    }
    
    LOG_INF("INT GPIO configured: pin=%d, flags=0x%x", 
            cfg->int_gpio.pin, cfg->int_gpio.dt_flags);
}

/* 硬件初始化 */
static int cst816d_hardware_init(const struct device *dev)
{
    struct cst816d_data *data = dev->data;  // 添加：获取设备数据
    int ret;  // 添加：声明ret变量

    /* 存储设备指针和全局指针 */
    data->dev = dev;
    g_cst816d_data = data;
    
    /* 1. 硬件复位 */
    cst816d_hardware_reset(dev);
    
    /* 2. 配置INT引脚 */
    // cst816d_configure_int_pin(dev);
    
    /* 3. 等待芯片稳定 */
    k_msleep(100);
    
    /* 4. 读取芯片ID验证连接 */
    LOG_INF("Reading chip ID...");
    uint8_t chip_id;
    ret = cst816d_i2c_read_internal(dev, CST816D_REG_CHIP_ID, &chip_id, 1);
    if (ret == 0) {
        LOG_INF("SUCCESS! Chip ID: 0x%02X", chip_id);
    } else {
        LOG_ERR("Failed to read chip ID: %d", ret);
        /* 即使失败也继续，可能芯片需要更多时间 */
    }
    
    /* 5. 禁止自动睡眠 */
    LOG_INF("Disabling auto sleep...");
    ret = cst816d_i2c_write_internal(dev, CST816D_REG_AUTO_SLEEP, 0xFF);
    if (ret == 0) {
        LOG_INF("Auto sleep disabled");
    } else {
        LOG_WRN("Failed to disable auto sleep: %d", ret);
    }

    return 0;
}

/* 上报触摸事件 */
static void cst816d_report_touch(const struct device *dev, const struct cst816d_touch *touch)
{
    // struct cst816d_data *data = dev->data;

    // LOG_INF("=== Reporting touch ===");
    // LOG_INF("Touch state: touched=%d, x=%d, y=%d, gesture=%d", 
    //         touch->touched, touch->x, touch->y, touch->gesture);

    // if (touch->touched) {
    //     /* 上报绝对坐标 */
    //     input_report_abs(dev, INPUT_ABS_X, touch->x, false, K_FOREVER);
    //     input_report_abs(dev, INPUT_ABS_Y, touch->y, false, K_FOREVER);
        
    //     /* 上报触摸按下 */
    //     input_report_key(dev, INPUT_BTN_TOUCH, 1, true, K_FOREVER);
        
    //     // LOG_INF("Report touch: x=%d, y=%d", touch->x, touch->y);
    // } else {
    //     /* 上报触摸释放 */
    //     input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_FOREVER);
    //     LOG_INF("Report touch release");
    // }

    /* 上报pointing */
    cst816d_handle_abs_event(dev, touch->x, touch->y, touch->touched);
    
    /* 上报手势（如果有） */
    if (touch->gesture != CST816D_GESTURE_NONE) {
        LOG_INF("gesture: %d", touch->gesture);
        /* 上报原始手势 */
        input_report(dev, INPUT_EV_DEVICE, (uint16_t)touch->gesture, 0, true, K_FOREVER);
    }
}

#ifdef CONFIG_INPUT_CST816D_INTERRUPT
static void cst816d_isr_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    struct cst816d_data *data = CONTAINER_OF(cb, struct cst816d_data, int_gpio_cb);
    
    /* 提交工作到工作队列，避免在ISR中做耗时操作 */
    k_work_submit(&data->work);
}

static void cst816d_work_handler(struct k_work *work)
{
    struct cst816d_data *data = CONTAINER_OF(work, struct cst816d_data, work);
    const struct device *dev = data->dev;
    struct cst816d_touch touch = {0};
    
    /* 读取触摸数据 */
    uint8_t gesture_raw = 0;
    if (cst816d_i2c_read_internal(dev, CST816D_REG_GESTURE, &gesture_raw, 1) == 0) {
        /* 转换手势 */
        switch (gesture_raw) {
            case 0x01: touch.gesture = CST816D_GESTURE_SLIDE_DOWN; break;
            case 0x02: touch.gesture = CST816D_GESTURE_SLIDE_UP; break;
            case 0x03: touch.gesture = CST816D_GESTURE_SLIDE_LEFT; break;
            case 0x04: touch.gesture = CST816D_GESTURE_SLIDE_RIGHT; break;
            case 0x05: touch.gesture = CST816D_GESTURE_SINGLE_TAP; break;
            case 0x0B: touch.gesture = CST816D_GESTURE_DOUBLE_TAP; break;
            case 0x0C: touch.gesture = CST816D_GESTURE_LONG_PRESS; break;
            default:   touch.gesture = CST816D_GESTURE_NONE; break;
        }
    }
    
    /* 读取触摸点数 */
    uint8_t finger = 0;
    int ret = cst816d_i2c_read_internal(dev, CST816D_REG_FINGER, &finger, 1);

    if (ret == 0) {
        // LOG_INF(" finger: %d. (int mode)", finger);
        /* 读取成功，检查是否有触摸 */
        if (finger > 0) {
            uint8_t touch_data[4];
            if (cst816d_i2c_read_internal(dev, CST816D_REG_XPOS_H, touch_data, 4) == 0) {
                touch.touched = true;
                touch.x = ((touch_data[0] & 0x0F) << 8) | touch_data[1];
                touch.y = ((touch_data[2] & 0x0F) << 8) | touch_data[3];
                
                /* 应用坐标转换 */
                const struct cst816d_config *cfg = dev->config;
                cst816d_transform_coordinates(cfg, &touch.x, &touch.y);

                /* 保存坐标 */
                data->last_touch.x = touch.x;
                data->last_touch.y = touch.y;

                // LOG_INF("Touch: x=%d, y=%d. (int mode)", touch.x, touch.y);
            }
        } else {
            /* 没有触摸，可以设置相应状态 */
            touch.touched = false;
        }
    } else {
        /* 读取失败，处理错误 */
        LOG_ERR("Failed to read finger count");
    }
    
    /* 特殊处理：单击/双击 */
    if (gesture_raw == 0x05 || gesture_raw == 0x0B) {
        touch.touched = true;
        touch.x = data->last_touch.x;
        touch.y = data->last_touch.y;
        // LOG_INF("%s tap detected at (%d, %d) (int mode)", 
        //         gesture_raw == 0x05 ? "Single" : "Double", touch.x, touch.y);
    }

    /* 上报触摸事件 */
    cst816d_report_touch(dev, &touch);

    /* 更新所有数据 */
    data->last_touch = touch;
    
    /* 重新启用中断 */
    const struct cst816d_config *cfg = dev->config;
    if (cfg->int_gpio.port) {
        gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_EDGE_FALLING);
    }
}

static int cst816d_init_interrupt(const struct device *dev)
{
    const struct cst816d_config *cfg = dev->config;
    struct cst816d_data *data = dev->data;
    int ret;
    
    if (!cfg->int_gpio.port) {
        LOG_ERR("No INT GPIO configured for interrupt mode");
        return -ENODEV;
    }
    
    /* 初始化工作项 */
    k_work_init(&data->work, cst816d_work_handler);
    
    /* 配置GPIO中断 */
    cst816d_configure_int_pin(dev);
    
    /* 设置中断回调 */
    gpio_init_callback(&data->int_gpio_cb, cst816d_isr_handler, BIT(cfg->int_gpio.pin));
    
    /* 添加回调并启用中断 */
    ret = gpio_add_callback(cfg->int_gpio.port, &data->int_gpio_cb);
    if (ret < 0) {
        LOG_ERR("Failed to add GPIO callback: %d", ret);
        return ret;
    }
    
    /* 配置中断触发方式（下降沿触发，因为ACTIVE_LOW） */
    ret = gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_EDGE_FALLING);
    if (ret < 0) {
        LOG_ERR("Failed to configure interrupt: %d", ret);
        return ret;
    }
    
    LOG_INF("Interrupt mode initialized");
    return 0;
}
#else
/* 轮询工作函数 */
static void cst816d_poll(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct cst816d_data *data = CONTAINER_OF(dwork, struct cst816d_data, poll_work);
    const struct device *dev = data->dev;
    struct cst816d_touch touch = {0};
    
    if (!data->initialized) {
        return;
    }
    
    /* 1. 读取手势 */
    uint8_t gesture_raw = 0;
    if (cst816d_i2c_read_internal(dev, CST816D_REG_GESTURE, &gesture_raw, 1) != 0) {
        /* I2C读取失败，跳过本次轮询 */
        goto reschedule;
    }
    
    /* 转换手势 */
    switch (gesture_raw) {
        case 0x01: touch.gesture = CST816D_GESTURE_SLIDE_DOWN; break;
        case 0x02: touch.gesture = CST816D_GESTURE_SLIDE_UP; break;
        case 0x03: touch.gesture = CST816D_GESTURE_SLIDE_LEFT; break;
        case 0x04: touch.gesture = CST816D_GESTURE_SLIDE_RIGHT; break;
        case 0x05: touch.gesture = CST816D_GESTURE_SINGLE_TAP; break;
        case 0x0B: touch.gesture = CST816D_GESTURE_DOUBLE_TAP; break;
        case 0x0C: touch.gesture = CST816D_GESTURE_LONG_PRESS; break;
        default:   touch.gesture = CST816D_GESTURE_NONE; break;
    }
    
    /* 2. 读取触摸状态 */
    uint8_t finger = 0;
    if (cst816d_i2c_read_internal(dev, CST816D_REG_FINGER, &finger, 1) == 0 && finger) {
        uint8_t touch_data[4];
        if (cst816d_i2c_read_internal(dev, CST816D_REG_XPOS_H, touch_data, 4) == 0) {
            touch.touched = true;
            touch.x = ((touch_data[0] & 0x0F) << 8) | touch_data[1];
            touch.y = ((touch_data[2] & 0x0F) << 8) | touch_data[3];

            /* 应用坐标转换 */
            const struct cst816d_config *cfg = dev->config;
            cst816d_transform_coordinates(cfg, &touch.x, &touch.y);

            /* 保存坐标 */
            data->last_touch.x = touch.x;
            data->last_touch.y = touch.y;
            
            LOG_INF("Touch: x=%d, y=%d", touch.x, touch.y);
        }
    }
    
    /* 3. 特殊处理：单击/双击 */
    if (gesture_raw == 0x05 || gesture_raw == 0x0B) {
        touch.touched = true;
        touch.x = data->last_touch.x;
        touch.y = data->last_touch.y;
        LOG_INF("%s tap detected at (%d, %d)", 
                gesture_raw == 0x05 ? "Single" : "Double", touch.x, touch.y);
    }
    
    /* 4. 上报触摸事件 */
    cst816d_report_touch(dev, &touch);

    /* 更新状态 */
    data->last_touch = touch;
    
reschedule:
    /* 重新调度 */
    k_work_schedule(dwork, K_MSEC(CST816D_POLL_MS));
}
#endif

/* 设置触摸模式 */
bool cst816d_set_mode(enum cst816d_touch_mode mode)
{
    if (!g_cst816d_data || !g_cst816d_data->initialized) {
        LOG_ERR("Device not initialized");
        return false;
    }
    
    const struct device *dev = g_cst816d_data->dev;
    int ret;
    
    LOG_INF("Setting touch mode: %d", mode);
    
    switch (mode) {
        case CST816D_MODE_POINT:
            /* 点模式：仅坐标检测 */
            LOG_INF("Setting point mode");
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_IRQ_CTL, CST816D_IRQ_CTL_POINT);
            if (ret != 0) {
                LOG_ERR("Failed to set IRQ_CTL: %d", ret);
                return false;
            }
            
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_NOR_SCAN_PER, 
                                           CST816D_NOR_SCAN_PER_DEFAULT);
            if (ret != 0) {
                LOG_ERR("Failed to set NOR_SCAN_PER: %d", ret);
                return false;
            }
            
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_IRQ_PULSE_WIDTH,
                                           CST816D_IRQ_PULSE_WIDTH_DEFAULT);
            if (ret != 0) {
                LOG_ERR("Failed to set IRQ_PULSE_WIDTH: %d", ret);
                return false;
            }
            break;
            
        case CST816D_MODE_MIXED:
            /* 混合模式 */
            LOG_INF("Setting mixed mode");
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_IRQ_CTL, CST816D_IRQ_CTL_MIXED);
            if (ret != 0) {
                LOG_ERR("Failed to set IRQ_CTL: %d", ret);
                return false;
            }
            break;
            
        case CST816D_MODE_GESTURE:
        default:
            /* 手势模式（默认） */
            LOG_INF("Setting gesture mode");
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_IRQ_CTL, CST816D_IRQ_CTL_GESTURE);
            if (ret != 0) {
                LOG_ERR("Failed to set IRQ_CTL: %d", ret);
                return false;
            }
            
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_NOR_SCAN_PER,
                                           CST816D_NOR_SCAN_PER_DEFAULT);
            if (ret != 0) {
                LOG_ERR("Failed to set NOR_SCAN_PER: %d", ret);
                return false;
            }
            
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_IRQ_PULSE_WIDTH,
                                           CST816D_IRQ_PULSE_WIDTH_DEFAULT);
            if (ret != 0) {
                LOG_ERR("Failed to set IRQ_PULSE_WIDTH: %d", ret);
                return false;
            }
            
            ret = cst816d_i2c_write_internal(dev, CST816D_REG_MOTION_MASK,
                                           CST816D_MOTION_MASK_DCLICK);
            if (ret != 0) {
                LOG_ERR("Failed to set MOTION_MASK: %d", ret);
                return false;
            }
            break;
    }
    
    LOG_INF("Touch mode set successfully: %d", mode);
    return true;
}

/* ========== Zephyr驱动初始化（使用设备树） ========== */

static int cst816d_driver_init(const struct device *dev)
{
    struct cst816d_data *data = dev->data;
    int ret;

    LOG_INF("=== CST816D Driver Initialization ===");  // 添加：开始日志

    /* 初始化 pointing 数据 */
    pointing_init_data(&data->pointing_data);

    /* 1. 检查配置 */
    ret = cst816d_check_config(dev);
    if (ret < 0) {
        return ret;
    }
    
    ret = cst816d_hardware_init(dev);
    if (ret < 0) {
        LOG_ERR("Hardware initialization failed: %d", ret);
        return ret;
    }

#ifdef CONFIG_INPUT_CST816D_INTERRUPT
    /* 中断模式初始化 */
	ret = cst816d_init_interrupt(dev);
	if (ret < 0) {
		LOG_ERR("Could not configure interrupt mode (%d)", ret);
		return ret;
	}
#else
    /* 初始化轮询工作 */
    k_work_init_delayable(&data->poll_work, cst816d_poll);
    k_work_schedule(&data->poll_work, K_MSEC(CST816D_POLL_MS));
#endif

    data->initialized = true;

    /* 使用调试宏定义的默认模式 */
    if (cst816d_set_mode(CST816D_DEBUG_DEFAULT_MODE)) {
        LOG_INF("Touch mode set successfully: %d", CST816D_DEBUG_DEFAULT_MODE);
    } else {
        LOG_WRN("Failed to set touch mode: %d", CST816D_DEBUG_DEFAULT_MODE);
    }

    LOG_INF("=== CST816D Driver Initialization Complete ===");
    return 0;
}

/* ========== 头文件API实现 ========== */

/* 初始化驱动 - 兼容性API（实际上驱动已通过设备树初始化） */
bool cst816d_init(const struct device *i2c_dev)
{
    LOG_INF("cst816d_init called (DT-based driver already initialized)");
    
    /* 如果驱动已经通过设备树初始化，直接返回成功 */
    if (g_cst816d_data && g_cst816d_data->initialized) {
        LOG_INF("Driver already initialized via device tree");
        return true;
    }
    
    /* 如果没有设备树初始化，尝试手动初始化 */
    LOG_WRN("Driver not initialized via device tree, attempting manual init...");
    
    /* 获取设备树设备 */
    const struct device *dev = DEVICE_DT_GET(DT_INST(0, hynitron_cst816d));
    if (!dev || !device_is_ready(dev)) {
        LOG_ERR("CST816D device not found or not ready");
        return false;
    }
    
    /* 驱动应该已经初始化，如果没有，返回失败 */
    return g_cst816d_data != NULL;
}

/* 读取触摸数据 */
bool cst816d_read_touch(struct cst816d_touch *touch)
{
    if (!g_cst816d_data || !touch || !g_cst816d_data->initialized) {
        return false;
    }
    
    /* 复制数据 */
    *touch = g_cst816d_data->last_touch;
    
    /* 读取后清除手势（避免重复触发） */
    bool had_gesture = (g_cst816d_data->last_touch.gesture != CST816D_GESTURE_NONE);
    g_cst816d_data->last_touch.gesture = CST816D_GESTURE_NONE;
    
    /* 返回true如果有触摸或有手势 */
    return g_cst816d_data->last_touch.touched || had_gesture;
}

/* 设备树实例定义 */
#define CST816D_DEFINE(index)                                              \
    static const struct cst816d_config cst816d_config_##index = {          \
        .i2c = I2C_DT_SPEC_INST_GET(index),                                \
        .rst_gpio = GPIO_DT_SPEC_INST_GET_OR(index, rst_gpios, {0}),       \
        .int_gpio = GPIO_DT_SPEC_INST_GET_OR(index, irq_gpios, {0}),       \
        .rotation = (uint16_t)DT_INST_PROP_OR(index, rotation, 0),         \
        .swap_xy = DT_INST_PROP_OR(index, swap_xy, false),                 \
        .invert_x = DT_INST_PROP_OR(index, invert_x, false),               \
        .invert_y = DT_INST_PROP_OR(index, invert_y, false),               \
        .width = (uint16_t)DT_INST_PROP_OR(index, width, 240),             \
        .height = (uint16_t)DT_INST_PROP_OR(index, height, 240),           \
    };                                                                     \
    static struct cst816d_data cst816d_data_##index;                       \
    DEVICE_DT_INST_DEFINE(index, cst816d_driver_init, NULL, &cst816d_data_##index, \
                          &cst816d_config_##index, POST_KERNEL,            \
                          CONFIG_INPUT_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(CST816D_DEFINE)