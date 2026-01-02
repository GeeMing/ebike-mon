#!/usr/bin/env python3
"""
将 img 目录中的所有图片调整为 36x36 像素
"""

from PIL import Image
import os

def resize_images(directory='img', target_size=(36, 36)):
    """
    将指定目录中的所有图片调整为指定尺寸

    Args:
        directory: 图片目录路径
        target_size: 目标尺寸 (width, height)
    """
    if not os.path.exists(directory):
        print(f"目录 {directory} 不存在")
        return

    # 支持的图片格式
    image_extensions = ('.png', '.jpg', '.jpeg', '.bmp', '.gif')

    # 遍历目录中的所有文件
    for filename in os.listdir(directory):
        if filename.lower().endswith(image_extensions):
            filepath = os.path.join(directory, filename)
            try:
                # 打开图片
                with Image.open(filepath) as img:
                    # 调整尺寸，使用高质量重采样
                    resized_img = img.resize(target_size, Image.Resampling.LANCZOS)

                    # 保存图片（覆盖原文件）
                    resized_img.save(filepath, optimize=True)
                    print(f"✓ 已调整: {filename} -> {target_size[0]}x{target_size[1]}")
            except Exception as e:
                print(f"✗ 处理 {filename} 时出错: {e}")

if __name__ == '__main__':
    print("开始调整图片尺寸为 36x36...")
    resize_images('img', (36, 36))
    print("完成！")
