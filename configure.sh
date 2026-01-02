cmake -S . -B _build  -G "MinGW Makefiles"  \
    -DCMAKE_C_COMPILER="D:/programs/TDM-GCC-64/bin/gcc.exe" \
    -DCMAKE_CXX_COMPILER="D:/programs/TDM-GCC-64/bin/g++.exe" \
    -DCMAKE_TOOLCHAIN_FILE="D:/programs/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="x64-mingw-dynamic"


# vcpkg 安装sdl
# vcpkg install sdl2:x64-mingw-dynamic

