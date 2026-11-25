# Linux x86_64 release build environment (tools/release/build_linux.sh).
# Native arm64 or amd64 Debian with the amd64 cross toolchain and multiarch
# development packages, so it runs fast on Apple Silicon (colima) as well as
# on x86_64 hosts. SDL2 is built from source as a static library.
FROM debian:bookworm

RUN dpkg --add-architecture amd64 && apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates cmake make ninja-build pkg-config python3 python3-pip python3-venv \
      crossbuild-essential-amd64 \
      libx11-dev:amd64 libxext-dev:amd64 libxrandr-dev:amd64 libxcursor-dev:amd64 libxfixes-dev:amd64 \
      libxi-dev:amd64 libxss-dev:amd64 libxkbcommon-dev:amd64 libwayland-dev:amd64 libdecor-0-dev:amd64 \
      libegl-dev:amd64 libgl-dev:amd64 libgles-dev:amd64 libdrm-dev:amd64 libgbm-dev:amd64 \
      libasound2-dev:amd64 libpulse-dev:amd64 libpipewire-0.3-dev:amd64 libdbus-1-dev:amd64 \
      libudev-dev:amd64 libibus-1.0-dev:amd64 \
      wayland-protocols \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/py && /opt/py/bin/pip install --no-cache-dir "capstone>=5.0" protobuf pillow
ENV PATH=/opt/py/bin:$PATH \
    PKG_CONFIG_LIBDIR=/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig
