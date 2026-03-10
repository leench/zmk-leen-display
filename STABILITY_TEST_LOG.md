# 稳定性测试修改记录 (Stability Test Log)

用于追踪为了解决 Dongle 偶尔死机问题而进行的临时修改。

## 2026-03-10
### 1. 禁用电池组件动画
- **文件**: `src/widgets/battery.c`
- **修改**: 
    - 将 `lv_bar_set_value` 的 `LV_ANIM_ON` 改为 `LV_ANIM_OFF`。
    - 注释掉了 `lv_obj_set_style_anim_time` 设置。
- **目的**: 减少 LVGL 动画引擎的负担，排除定时器或动画回调引起的死机。

### 2. 优化时钟组件刷新
- **文件**: `src/widgets/clock.c`
- **修改**: 
    - 注释掉 `clock_update_display` 函数中每秒调用的 `lv_obj_align_to`。
- **目的**: 避免每秒触发 LVGL 布局重新计算。由于使用的是 JetBrains Mono 等宽字体，"HH:MM" 的宽度是固定的，初始化时对齐一次即可。
