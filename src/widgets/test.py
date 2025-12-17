#!/usr/bin/env python3
"""
生成两帧简单点的 LVGL 图片 C 数组，用于 bongo_cat widget 动画
"""

import sys

def generate_lvgl_dot_image(name, width=16, height=16, dot_pos=(0,0)):
    """
    name: 图片名称
    width, height: 图片尺寸
    dot_pos: 点的位置 (x, y)
    """
    # 颜色索引：0=透明, 1=白色
    color0 = (0x00,0x00,0x00,0x00)  # 透明
    color1 = (0xFF,0xFF,0xFF,0xFF)  # 白色

    # 图片像素索引数组
    pixels = []
    for y in range(height):
        for x in range(width):
            if (x, y) == dot_pos:
                pixels.append(1)
            else:
                pixels.append(0)

    # 输出 C 数组
    c = []
    c.append(f"const uint8_t {name}_map[] = {{")
    c.append(f"  /* Colors */")
    c.append(f"  0x{color0[0]:02X},0x{color0[1]:02X},0x{color0[2]:02X},0x{color0[3]:02X}, /* index 0 */")
    c.append(f"  0x{color1[0]:02X},0x{color1[1]:02X},0x{color1[2]:02X},0x{color1[3]:02X}, /* index 1 */")
    c.append(f"  /* Pixel indices */")
    # 每行像素输出
    for y in range(height):
        row = pixels[y*width:(y+1)*width]
        row_str = ", ".join(str(v) for v in row)
        c.append(f"  {row_str},")
    c.append("};\n")
    return "\n".join(c)

def main():
    frame1 = generate_lvgl_dot_image("bongo_cat_left1", dot_pos=(2,2))
    frame2 = generate_lvgl_dot_image("bongo_cat_right1", dot_pos=(13,13))

    print("// ---------- 自动生成的两帧简单点图片 ----------\n")
    print("#include <lvgl.h>\n")
    print(frame1)
    print(frame2)
    print("LV_IMG_DECLARE(bongo_cat_left1);")
    print("LV_IMG_DECLARE(bongo_cat_right1);")

if __name__ == "__main__":
    main()
