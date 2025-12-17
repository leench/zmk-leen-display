import re

INPUT_FILE = "bongo_cat_images.c"
OUTPUT_FILE = "bongo_cat_images_inverted.c"

# 读取文件
with open(INPUT_FILE, "r") as f:
    content = f.read()

# 匹配数组声明开头和内容
array_match = re.search(r"uint8_t\s+\w+_map\s*\[\]\s*=\s*\{(.*?)\};", content, re.S)
if not array_match:
    print("没有找到数组！")
    exit(1)

array_content = array_match.group(1)

# 提取十六进制数字
hex_numbers = re.findall(r"0x[0-9a-fA-F]{1,2}", array_content)

if len(hex_numbers) < 8:
    print("数组长度太短，无法判断调色板！")
    exit(1)

# 假设前两个颜色为调色板，每个颜色 4 字节 RGBA
palette_bytes = hex_numbers[:8]
pixel_bytes = hex_numbers[8:]

# 解析调色板
def hex_to_int(h):
    return int(h, 16)

palette = []
for i in range(0, len(palette_bytes), 4):
    r = hex_to_int(palette_bytes[i])
    g = hex_to_int(palette_bytes[i+1])
    b = hex_to_int(palette_bytes[i+2])
    a = hex_to_int(palette_bytes[i+3])
    palette.append([r, g, b, a])

# 反色处理
for i, color in enumerate(palette):
    r, g, b, a = color
    if (r, g, b) == (255, 255, 255):
        # 白色改为透明
        palette[i] = [0, 0, 0, 0]
    elif (r, g, b) == (0, 0, 0):
        # 黑色改为白色
        palette[i] = [255, 255, 255, a]

# 生成新的数组内容
new_array_bytes = []
for color in palette:
    new_array_bytes.extend([f"0x{v:02x}" for v in color])
new_array_bytes.extend(pixel_bytes)  # 像素索引保持不变

# 按每行 8 个字节格式化
lines = []
for i in range(0, len(new_array_bytes), 8):
    line = ", ".join(new_array_bytes[i:i+8])
    lines.append(f"  {line},")

new_array_str = "\n".join(lines)

# 替换原数组
new_content = re.sub(r"(uint8_t\s+\w+_map\s*\[\]\s*=\s*\{).*?(\};)", 
                     r"\1\n" + new_array_str + r"\n\2",
                     content, flags=re.S)

# 写入新文件
with open(OUTPUT_FILE, "w") as f:
    f.write(new_content)

print(f"处理完成，输出文件: {OUTPUT_FILE}")