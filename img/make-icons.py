#!/usr/bin/env python3
"""
LVGL 图标生成脚本
用于将 img 目录下的 PNG 图片转换为 icons.c 和 icons.h 文件

使用方法:
    python3 make-icons.py [选项]

示例:
    # 使用默认参数 (ARGB8888, 无压缩)
    python3 make-icons.py

    # 使用 RGB565 格式，LZ4 压缩
    python3 make-icons.py --cf RGB565 --compress LZ4

    # 指定输出文件路径
    python3 make-icons.py --output-c ../src/icons.c --output-h ../src/icons.h

依赖:
    需要安装以下 Python 包:
    - pypng: pip3 install pypng
    - lz4: pip3 install lz4 (如果使用 LZ4 压缩)
"""

import os
import sys
from pathlib import Path
from typing import List, Tuple

# 添加 simulator/lvgl/scripts 目录到路径，以便导入 LVGLImage
script_dir = Path(__file__).parent
lvgl_scripts_dir = script_dir.parent / "simulator" / "lvgl" / "scripts"
sys.path.insert(0, str(lvgl_scripts_dir))

try:
    from LVGLImage import (
        LVGLImage, ColorFormat, CompressMethod,
        PNGConverter, OutputFormat, LVGLCompressData
    )
except ImportError as e:
    print(f"错误: 无法导入 LVGLImage 模块: {e}")
    print("请确保 simulator/lvgl/scripts/LVGLImage.py 存在")
    sys.exit(1)


class IconsGenerator:
    """图标生成器类"""

    def __init__(self,
                 img_dir: str = None,
                 output_c: str = None,
                 output_h: str = None,
                 color_format: str = "ARGB8888",
                 compress: str = "NONE",
                 align: int = 1,
                 premultiply: bool = False,
                 background: int = 0x000000,
                 rgb565_dither: bool = False):
        """
        初始化图标生成器

        参数:
            img_dir: 图片目录路径，默认为脚本所在目录
            output_c: 输出的 C 文件路径，默认为 img_dir/icons.c
            output_h: 输出的 H 文件路径，默认为 img_dir/icons.h
            color_format: 颜色格式 (ARGB8888, RGB565, I8, 等)
            compress: 压缩方式 (NONE, RLE, LZ4)
            align: 对齐字节数
            premultiply: 是否预乘 alpha
            background: 背景颜色 (用于无 alpha 格式)
            rgb565_dither: RGB565 是否使用抖动
        """
        if img_dir is None:
            img_dir = script_dir
        self.img_dir = Path(img_dir)

        if output_c is None:
            output_c = self.img_dir / "icons.c"
        if output_h is None:
            output_h = self.img_dir / "icons.h"
        self.output_c = Path(output_c)
        self.output_h = Path(output_h)

        # 解析颜色格式
        try:
            self.cf = ColorFormat[color_format]
        except KeyError:
            print(f"错误: 不支持的颜色格式: {color_format}")
            print(f"支持的格式: {', '.join([e.name for e in ColorFormat if e != ColorFormat.UNKNOWN])}")
            sys.exit(1)

        # 解析压缩方式
        try:
            self.compress = CompressMethod[compress]
        except KeyError:
            print(f"错误: 不支持的压缩方式: {compress}")
            print("支持的格式: NONE, RLE, LZ4")
            sys.exit(1)

        self.align = align
        self.premultiply = premultiply
        self.background = background
        self.rgb565_dither = rgb565_dither

        # 存储转换后的图片
        self.images: List[Tuple[str, LVGLImage]] = []

    def find_images(self) -> List[Path]:
        """查找所有 PNG 图片文件"""
        # 使用 set 去重，避免在大小写不敏感的文件系统上重复
        png_files = set(self.img_dir.glob("*.png"))
        png_files.update(self.img_dir.glob("*.PNG"))
        return sorted(png_files)

    def convert_images(self):
        """转换所有图片"""
        png_files = self.find_images()

        if not png_files:
            print(f"警告: 在 {self.img_dir} 中未找到 PNG 图片")
            return

        print(f"找到 {len(png_files)} 个图片文件:")
        for f in png_files:
            print(f"  - {f.name}")

        print(f"\n转换参数:")
        print(f"  颜色格式: {self.cf.name}")
        print(f"  压缩方式: {self.compress.name}")
        print(f"  对齐: {self.align} 字节")
        print(f"  预乘 alpha: {self.premultiply}")
        print(f"  背景色: 0x{self.background:06x}")
        print(f"  RGB565 抖动: {self.rgb565_dither}")
        print()

        # 转换每个图片
        for png_file in png_files:
            try:
                print(f"转换: {png_file.name}...", end=" ")
                img = LVGLImage().from_png(
                    str(png_file),
                    self.cf,
                    background=self.background,
                    rgb565_dither=self.rgb565_dither
                )
                img.adjust_stride(align=self.align)

                if self.premultiply:
                    img.premultiply()

                # 生成变量名（从文件名）
                varname = png_file.stem.replace("-", "_").replace(".", "_")
                self.images.append((varname, img))
                print(f"完成 ({img.w}x{img.h}, {img.data_len} 字节)")
            except Exception as e:
                print(f"失败: {e}")
                continue

        print(f"\n成功转换 {len(self.images)} 个图片")

    def write_c_file(self):
        """写入 C 文件"""
        print(f"\n生成 C 文件: {self.output_c}")

        with open(self.output_c, 'w', encoding='utf-8') as f:
            # 写入文件头
            f.write("""/*
 * LVGL 图标资源文件
 * 此文件由 make-icons.py 自动生成，请勿手动编辑
 */
#include "lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

""")

            # 写入每个图片的数据
            for varname, img in self.images:
                macro = f"LV_ATTRIBUTE_{varname.upper()}"
                f.write(f"#ifndef {macro}\n")
                f.write(f"#define {macro}\n")
                f.write(f"#endif\n\n")

                flags = "0"
                if self.compress != CompressMethod.NONE:
                    flags += " | LV_IMAGE_FLAGS_COMPRESSED"
                if self.premultiply:
                    flags += " | LV_IMAGE_FLAGS_PREMULTIPLIED"

                # 写入数据数组
                f.write(f"static const\n")
                f.write(f"LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST {macro}\n")
                f.write(f"uint8_t {varname}_map[] = {{\n")

                # 写入二进制数据
                self._write_binary_data(f, img)

                f.write(f"}};\n\n")

                # 写入图片描述结构
                f.write(f"const lv_image_dsc_t {varname} = {{\n")
                f.write(f"  .header = {{\n")
                f.write(f"    .magic = LV_IMAGE_HEADER_MAGIC,\n")
                f.write(f"    .cf = LV_COLOR_FORMAT_{img.cf.name},\n")
                f.write(f"    .flags = {flags},\n")
                f.write(f"    .w = {img.w},\n")
                f.write(f"    .h = {img.h},\n")
                f.write(f"    .stride = {img.stride},\n")
                f.write(f"    .reserved_2 = 0,\n")
                f.write(f"  }},\n")
                f.write(f"  .data_size = sizeof({varname}_map),\n")
                f.write(f"  .data = {varname}_map,\n")
                f.write(f"  .reserved = NULL,\n")
                f.write(f"}};\n\n")

    def _write_binary_data(self, f, img: LVGLImage):
        """写入二进制数据到文件"""
        # 根据压缩方式获取数据
        if self.compress != CompressMethod.NONE:
            # 压缩数据
            compressed = LVGLCompressData(img.cf, self.compress, img.data)
            data = compressed.compressed
        else:
            # 未压缩数据
            data = img.data

        # 写入数据，每行16个字节
        stride = 16
        for i, v in enumerate(data):
            if i % stride == 0:
                f.write("    ")
            f.write(f"0x{v:02x},")
            if (i + 1) % stride == 0:
                f.write("\n")
        if len(data) % stride != 0:
            f.write("\n")

    def write_h_file(self):
        """写入 H 文件"""
        print(f"生成 H 文件: {self.output_h}")

        with open(self.output_h, 'w', encoding='utf-8') as f:
            f.write('#pragma once\n')
            f.write('#include "lvgl.h"\n\n')

            # 写入每个图片的声明（去重）
            seen = set()
            for varname, img in self.images:
                if varname not in seen:
                    f.write(f"LV_IMAGE_DECLARE({varname});\n")
                    seen.add(varname)

    def generate(self):
        """生成所有文件"""
        self.convert_images()

        if not self.images:
            print("没有可用的图片，退出")
            return

        self.write_c_file()
        self.write_h_file()
        print(f"\n完成! 已生成:")
        print(f"  - {self.output_c}")
        print(f"  - {self.output_h}")


def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(
        description='LVGL 图标生成工具 - 将 PNG 图片转换为 icons.c 和 icons.h'
    )
    parser.add_argument(
        '--img-dir',
        type=str,
        default=None,
        help='图片目录路径 (默认: 脚本所在目录)'
    )
    parser.add_argument(
        '--output-c',
        type=str,
        default=None,
        help='输出的 C 文件路径 (默认: img_dir/icons.c)'
    )
    parser.add_argument(
        '--output-h',
        type=str,
        default=None,
        help='输出的 H 文件路径 (默认: img_dir/icons.h)'
    )
    parser.add_argument(
        '--cf',
        '--color-format',
        type=str,
        default='ARGB8888',
        choices=['L8', 'I1', 'I2', 'I4', 'I8', 'A1', 'A2', 'A4', 'A8', 'AL88',
                 'ARGB8888', 'XRGB8888', 'RGB565', 'RGB565_SWAPPED', 'RGB565A8',
                 'ARGB8565', 'RGB888', 'ARGB8888_PREMULTIPLIED'],
        help='颜色格式 (默认: ARGB8888)'
    )
    parser.add_argument(
        '--compress',
        type=str,
        default='NONE',
        choices=['NONE', 'RLE', 'LZ4'],
        help='压缩方式 (默认: NONE)'
    )
    parser.add_argument(
        '--align',
        type=int,
        default=1,
        help='对齐字节数 (默认: 1)'
    )
    parser.add_argument(
        '--premultiply',
        action='store_true',
        help='预乘 alpha 通道'
    )
    parser.add_argument(
        '--background',
        type=lambda x: int(x, 0),
        default=0x000000,
        help='背景颜色 (用于无 alpha 格式, 默认: 0x000000)'
    )
    parser.add_argument(
        '--rgb565-dither',
        action='store_true',
        help='RGB565 格式使用抖动'
    )

    args = parser.parse_args()

    # 创建生成器并生成文件
    generator = IconsGenerator(
        img_dir=args.img_dir,
        output_c=args.output_c,
        output_h=args.output_h,
        color_format=args.cf,
        compress=args.compress,
        align=args.align,
        premultiply=args.premultiply,
        background=args.background,
        rgb565_dither=args.rgb565_dither
    )

    generator.generate()


if __name__ == "__main__":
    main()
