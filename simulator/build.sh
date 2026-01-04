#!/bin/bash
# Kconfig 模式构建脚本
# 注意：如果修改了 kconfig 配置，需要先运行 configure.sh 重新配置

# 进入构建目录并构建
cd _build && cmake .. && cmake --build . -- -j12