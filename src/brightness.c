#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <math.h>
#include <stdlib.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// 简化配置检查
#if CONFIG_DONGLE_SCREEN_MIN_BRIGHTNESS > CONFIG_DONGLE_SCREEN_MAX_BRIGHTNESS
#error "DONGLE_SCREEN_MIN_BRIGHTNESS must be less than or equal to DONGLE_SCREEN_MAX_BRIGHTNESS!"
#endif

#if CONFIG_DONGLE_SCREEN_DEFAULT_BRIGHTNESS < CONFIG_DONGLE_SCREEN_MIN_BRIGHTNESS
#error "DONGLE_SCREEN_DEFAULT_BRIGHTNESS can't be smaller than MIN brightness value!"
#endif

#if CONFIG_DONGLE_SCREEN_DEFAULT_BRIGHTNESS > CONFIG_DONGLE_SCREEN_MAX_BRIGHTNESS
#error "DONGLE_SCREEN_DEFAULT_BRIGHTNESS can't be greater than MAX brightness value!"
#endif

#define BRIGHTNESS_STEP 1
#define BRIGHTNESS_FADE_DURATION_MS 500

static const struct device *pwm_leds_dev = DEVICE_DT_GET_ONE(pwm_leds);
#define DISP_BL DT_NODE_CHILD_IDX(DT_NODELABEL(disp_bl))

static int64_t last_activity = 0;
static uint8_t max_brightness = CONFIG_DONGLE_SCREEN_MAX_BRIGHTNESS;
static uint8_t min_brightness = CONFIG_DONGLE_SCREEN_MIN_BRIGHTNESS;
static uint8_t current_brightness = CONFIG_DONGLE_SCREEN_DEFAULT_BRIGHTNESS;

static int8_t brightness_modifier = 0; // 亮度调节器，初始为0

static bool screen_on = true;
static bool off_through_modifier = false; // 标记是否通过调节器关闭屏幕

static uint8_t clamp_brightness(int8_t value)
{
    if (value > max_brightness)
    {
        LOG_WRN("CLAMPED: Screen brightness %d would be over %d", value, max_brightness);
        return max_brightness;
    }
    if (value < min_brightness)
    {
        LOG_WRN("CLAMPED: Screen brightness %d would be under %d", value, min_brightness);
        return min_brightness;
    }
    return value;
}

static void apply_brightness(uint8_t value)
{
    led_set_brightness(pwm_leds_dev, DISP_BL, value);
    LOG_INF("Screen brightness set to %d", value);
}

static int8_t calculate_safe_modifier_change(int8_t desired_change)
{
    int16_t current_effective = current_brightness + brightness_modifier;
    int16_t desired_effective = current_effective + desired_change;

    // 根据变化方向确定边界
    int16_t boundary = (desired_change > 0) ? max_brightness : min_brightness;

    // 检查期望变化是否在边界内
    if ((desired_change > 0 && desired_effective <= boundary) ||
        (desired_change < 0 && desired_effective >= boundary))
    {
        return desired_change; // 完整变化是安全的
    }

    // 计算朝向边界的最大安全变化
    int16_t safe_change = boundary - current_effective;

    // 确保不会返回错误方向的变化
    if ((desired_change > 0 && safe_change > 0) || (desired_change < 0 && safe_change < 0))
    {
        return (int8_t)safe_change;
    }

    return 0; // 没有安全变化可能
}

static bool should_screen_turn_off(void)
{
    return (current_brightness + brightness_modifier) < min_brightness;
}

static bool should_screen_turn_on(void)
{
    return (current_brightness + brightness_modifier) > min_brightness;
}

// 渐变请求结构
struct fade_request_t
{
    uint8_t from; // 起始亮度
    uint8_t to;   // 目标亮度
};

#define FADE_QUEUE_SIZE 4
K_MSGQ_DEFINE(fade_msgq, sizeof(struct fade_request_t), FADE_QUEUE_SIZE, 4);

// 立方缓动函数
static float ease_in_out(float t)
{
    if (t < 0.5f)
        return 4.0f * t * t * t;
    float f = -2.0f * t + 2.0f;
    return 1.0f - (f * f * f) / 2.0f;
}

// 渐变处理线程
void fade_thread(void)
{
    struct fade_request_t req;

    while (1)
    {
        if (k_msgq_get(&fade_msgq, &req, K_FOREVER) == 0)
        {
            // 如果亮度差异太小，跳过动画
            if (req.from == req.to || abs(req.to - req.from) <= 1)
            {
                apply_brightness(req.to);
                continue;
            }

            // 计算亮度差异和步数
            int diff = abs(req.to - req.from);
            int steps = CLAMP(diff * 2, 6, 32);
            int total_duration_ms = CLAMP(diff * 20, 500, 1000);
            int delay_us = (total_duration_ms * 1000) / steps;

            uint8_t last_applied = 255;

            // 使用缓动函数插值
            for (int i = 0; i <= steps; i++)
            {
                float t = (float)i / steps;
                float eased = ease_in_out(t);
                float interpolated = req.from + (req.to - req.from) * eased;
                uint8_t brightness = (uint8_t)(interpolated + 0.5f);

                if (brightness != last_applied)
                {
                    apply_brightness(brightness);
                    last_applied = brightness;
                }

                k_usleep(delay_us);
            }

            // 确保最终设置目标值
            if (last_applied != req.to)
            {
                apply_brightness(req.to);
            }
        }
    }
}

K_THREAD_DEFINE(fade_tid, 768, fade_thread, NULL, NULL, NULL, 6, 0, 0);

// 提交渐变请求
static void fade_to_brightness(uint8_t from, uint8_t to)
{
    struct fade_request_t req = {.from = from, .to = to};
    k_msgq_purge(&fade_msgq);
    k_msgq_put(&fade_msgq, &req, K_NO_WAIT);
}

// 屏幕开关控制
static void screen_set_on(bool on)
{
    if (on && !screen_on)
    {
        // 检查是否需要调整亮度才能打开屏幕
        if (should_screen_turn_off())
        {
            // 将亮度调整到最小值以上
            brightness_modifier = min_brightness - current_brightness + 1;
            LOG_DBG("SCREEN TURN ON: Adjusted modifier to ensure screen can turn on: %d", brightness_modifier);
        }

        fade_to_brightness(0, clamp_brightness(current_brightness + brightness_modifier));
        screen_on = true;
        off_through_modifier = false;
        LOG_INF("Screen on (smooth)");
    }
    else if (!on && screen_on)
    {
        fade_to_brightness(clamp_brightness(current_brightness + brightness_modifier), 0);
        screen_on = false;
        LOG_INF("Screen off (smooth)");
    }
    else
    {
        LOG_DBG("Screen state is already %s, no action taken.", on ? "on" : "off");
    }
}

// 空闲超时线程（使用CONFIG_ZMK_IDLE_TIMEOUT）
#if CONFIG_ZMK_IDLE_TIMEOUT > 0

void screen_idle_thread(void)
{
    while (1)
    {
        if (screen_on || (!screen_on && off_through_modifier))
        {
            int64_t now = k_uptime_get();
            int64_t elapsed_ms = now - last_activity;
            int64_t off_timeout_ms = (int64_t)CONFIG_ZMK_IDLE_TIMEOUT;

            if (elapsed_ms >= off_timeout_ms)
            {
                screen_set_on(false);
                k_sleep(K_FOREVER);
            }
            else
            {
                int64_t sleep_time = off_timeout_ms - elapsed_ms;
                if (sleep_time > 100)
                {
                    k_sleep(K_MSEC(100));
                }
                else if (sleep_time > 0)
                {
                    k_sleep(K_MSEC(sleep_time));
                }
                else
                {
                    k_sleep(K_MSEC(10));
                }
            }
        }
        else
        {
            k_sleep(K_FOREVER);
        }
    }
}

K_THREAD_DEFINE(screen_idle_tid, 512, screen_idle_thread, NULL, NULL, NULL, 7, 0, 0);

void brightness_wake_screen_on_reconnect(void)
{
    if (!screen_on)
    {
        LOG_INF("Peripheral reconnected, waking screen");
        screen_set_on(true);
        last_activity = k_uptime_get();
        k_wakeup(screen_idle_tid);
    }
    else
    {
        LOG_DBG("Peripheral reconnected but screen already on");
    }
}

#endif // CONFIG_ZMK_IDLE_TIMEOUT > 0

// 键盘亮度控制
#if CONFIG_DONGLE_SCREEN_BRIGHTNESS_KEYBOARD_CONTROL

static void increase_brightness(void)
{
    LOG_DBG("Current brightness: %d, current modifier: %d", current_brightness, brightness_modifier);

    int8_t safe_increase = calculate_safe_modifier_change(CONFIG_DONGLE_SCREEN_BRIGHTNESS_STEP);

    if (safe_increase > 0)
    {
        brightness_modifier += safe_increase;
        LOG_DBG("Brightness modifier increased by %d to %d", safe_increase, brightness_modifier);
        
        if (screen_on)
        {
            uint8_t target = clamp_brightness(current_brightness + brightness_modifier);
            uint8_t current_effective = clamp_brightness(current_brightness + (brightness_modifier - safe_increase));
            fade_to_brightness(current_effective, target);
        }

        // 检查是否应该打开屏幕
        if (should_screen_turn_on() && off_through_modifier)
        {
            LOG_INF("Brightness sufficient to turn screen on");
            screen_set_on(true);
        }
    }
    else
    {
        LOG_DBG("Brightness modifier cannot be increased further (at maximum)");
    }
}

static void decrease_brightness(void)
{
    LOG_DBG("Current brightness: %d, current modifier: %d", current_brightness, brightness_modifier);

    int8_t safe_decrease = calculate_safe_modifier_change(-CONFIG_DONGLE_SCREEN_BRIGHTNESS_STEP);

    if (safe_decrease < 0)
    {
        brightness_modifier += safe_decrease;
        LOG_DBG("Brightness modifier decreased by %d to %d", -safe_decrease, brightness_modifier);
        
        if (screen_on)
        {
            uint8_t target = clamp_brightness(current_brightness + brightness_modifier);
            uint8_t current_effective = clamp_brightness(current_brightness + (brightness_modifier - safe_decrease));
            fade_to_brightness(current_effective, target);
        }

        // 检查是否应该关闭屏幕
        if (should_screen_turn_off())
        {
            LOG_INF("Brightness too low, turning screen off");
            off_through_modifier = true;
            screen_set_on(false);
        }
    }
    else
    {
        LOG_DBG("Brightness modifier cannot be decreased further (at minimum)");
    }
}

#endif // CONFIG_DONGLE_SCREEN_BRIGHTNESS_KEYBOARD_CONTROL

// 按键事件监听器
#if CONFIG_ZMK_IDLE_TIMEOUT > 0 || CONFIG_DONGLE_SCREEN_BRIGHTNESS_KEYBOARD_CONTROL

static int key_listener(const zmk_event_t *eh)
{
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev && ev->state)
    {
        LOG_DBG("Key pressed: keycode=%d", ev->keycode);

#if CONFIG_DONGLE_SCREEN_BRIGHTNESS_KEYBOARD_CONTROL
        if (ev->keycode == CONFIG_DONGLE_SCREEN_BRIGHTNESS_UP_KEYCODE)
        {
            LOG_INF("Brightness UP key recognized!");
            increase_brightness();
            return 0;
        }
        else if (ev->keycode == CONFIG_DONGLE_SCREEN_BRIGHTNESS_DOWN_KEYCODE)
        {
            LOG_INF("Brightness DOWN key recognized!");
            decrease_brightness();
            return 0;
        }
        else if (ev->keycode == CONFIG_DONGLE_SCREEN_TOGGLE_KEYCODE)
        {
            LOG_INF("Toggle screen key recognized!");
            // 切换屏幕开关
            if (screen_on)
            {
                off_through_modifier = true;
                screen_set_on(false);
            }
            else
            {
                screen_set_on(true);
            }
            return 0;
        }
#endif
    }

#if CONFIG_ZMK_IDLE_TIMEOUT > 0
    last_activity = k_uptime_get();
    if (!screen_on && !off_through_modifier)
    {
        screen_set_on(true);
        k_wakeup(screen_idle_tid);
    }
#else
    // 没有空闲线程：直接打开屏幕
    if (!screen_on)
    {
        screen_set_on(true);
    }
#endif
    return 0;
}

ZMK_LISTENER(screen_idle, key_listener);
ZMK_SUBSCRIPTION(screen_idle, zmk_keycode_state_changed);
ZMK_SUBSCRIPTION(screen_idle, zmk_layer_state_changed);

#endif

// 初始化
static int init_fixed_brightness(void)
{
    // 设置初始亮度
    uint8_t initial_brightness = clamp_brightness(current_brightness + brightness_modifier);
    apply_brightness(initial_brightness);
    
    last_activity = k_uptime_get();
    
#if CONFIG_ZMK_IDLE_TIMEOUT > 0
    // 启动时唤醒空闲线程
    k_wakeup(screen_idle_tid);
#else
    LOG_INF("Screen idle timeout disabled");
#endif
    
    return 0;
}

SYS_INIT(init_fixed_brightness, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);