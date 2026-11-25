# Cross-compile the desktop build for 64-bit Windows with MinGW-w64
# (Homebrew mingw-w64 or a Linux distribution's package):
#   cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/MinGW64Toolchain.cmake \
#         -DCMAKE_PREFIX_PATH=<SDL2-devel-mingw>/x86_64-w64-mingw32 -DMMU_GAME_DIR=<game folder>
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
