#!/bin/sh
# Build the Linux x86_64 release binary in Docker (tools/release/linux.Dockerfile).
#
#   tools/release/build_linux.sh <game dir> <SDL2 source dir> [imgui dir]
#
# Output: build-release/linux/recomp/mmu_recomp (SDL2 linked statically).
# The game folder is mounted read-only; it is only read at build time (lift.py,
# font and title-prompt extraction).
set -eu
repo=$(cd "$(dirname "$0")/../.." && pwd)
game=$(cd "$1" && pwd)
sdl=$(cd "$2" && pwd)
imgui=$(cd "${3:-$repo/../imgui}" && pwd)

docker build -t mmu-recomp-linux -f "$repo/tools/release/linux.Dockerfile" "$repo/tools/release"
docker run --rm \
  -v "$repo:/src" -v "$game:/game:ro" -v "$sdl:/sdl-src:ro" -v "$imgui:/imgui:ro" \
  mmu-recomp-linux sh -euc '
    tc=/src/cmake/LinuxAmd64Toolchain.cmake
    out=/src/build-release/linux
    if [ ! -f "$out-sdl/lib/libSDL2.a" ]; then
      cmake -S /sdl-src -B /tmp/sdl -G Ninja -DCMAKE_TOOLCHAIN_FILE=$tc -DCMAKE_BUILD_TYPE=Release \
        -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF -DSDL_STATIC_PIC=ON -DCMAKE_INSTALL_PREFIX="$out-sdl"
      cmake --build /tmp/sdl
      cmake --install /tmp/sdl
    fi
    cmake -S /src -B "$out" -G Ninja -DCMAKE_TOOLCHAIN_FILE=$tc -DCMAKE_BUILD_TYPE=Release \
      -DSDL2_DIR="$out-sdl/lib/cmake/SDL2" -DMMU_GAME_DIR=/game -DMMU_ASSET_DIR=/game/Assets \
      -DMMU_IMGUI_DIR=/imgui -DMMU_PYTHON=/opt/py/bin/python3
    cmake --build "$out" --target mmu_recomp
    x86_64-linux-gnu-strip -o "$out/mmu_recomp" "$out/recomp/mmu_recomp"
  '
ls -l "$repo/build-release/linux/mmu_recomp"
