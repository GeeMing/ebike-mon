cmake -S . -B _build  -G "MinGW Makefiles"  \
    -DCMAKE_C_COMPILER="D:/programs/TDM-GCC-64/bin/gcc.exe" \
    -DCMAKE_CXX_COMPILER="D:/programs/TDM-GCC-64/bin/g++.exe" \
    -DCMAKE_TOOLCHAIN_FILE="D:/programs/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="x64-mingw-dynamic" \
    -DUSE_FREERTOS=ON \
    -DLV_BUILD_USE_KCONFIG=ON \
    -DLV_BUILD_DEFCONFIG_PATH="config/lvgl.config" \
    -DCONFIG_LV_BUILD_EXAMPLES=OFF \
    -DCONFIG_LV_BUILD_DEMOS=OFF


# vcpkg 安装sdl
# vcpkg install sdl2:x64-mingw-dynamic


# 现在使用Kconfig 代替 lv_conf.h
# 命令行使用 menuconfig
# 图形界面使用 guiconfig

# 安装：
# pip install kconfiglib