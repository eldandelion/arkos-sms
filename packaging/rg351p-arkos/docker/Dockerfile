# ArkOS-compatible (glibc 2.28 <= ArkOS 2.30, GLIBCXX 3.4.25 <= ArkOS 3.4.27)
# native ARM64 build environment for PortMaster / Anbernic RG351P (RK3326)
FROM --platform=linux/arm64 arm64v8/debian:buster

ENV DEBIAN_FRONTEND=noninteractive

# Debian Buster moved to archive.debian.org
RUN sed -i 's|http://deb.debian.org/debian|http://archive.debian.org/debian|g' /etc/apt/sources.list && \
    sed -i 's|http://security.debian.org/debian-security|http://archive.debian.org/debian-security|g' /etc/apt/sources.list && \
    sed -i '/buster-updates/d' /etc/apt/sources.list && \
    echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99no-check-valid-until && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      binutils \
      pkg-config \
      python3 \
      patch \
      git \
      ninja-build \
      curl \
      ca-certificates \
      xz-utils \
      zip \
      unzip \
      libegl1-mesa-dev \
      libgles2-mesa-dev \
      libdrm-dev \
      libgbm-dev \
      libasound2-dev \
      libudev-dev \
      libdbus-1-dev \
      libx11-dev \
      libxext-dev && \
    rm -rf /var/lib/apt/lists/*

# Install CMake 3.28.6 (sms-pc-port requires CMake >= 3.20; Buster ships 3.13)
ARG CMAKE_VERSION=3.28.6
RUN curl -fsSL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-aarch64.tar.gz" \
    | tar -xz --strip-components=1 -C /usr/local

# Build SDL2 2.26.2 from source into /usr (matches PortMaster official build chroot;
# used for compile-time headers & link-time SONAME libSDL2-2.0.so.0 only — NEVER bundled!)
ARG SDL2_VERSION=2.26.2
RUN curl -fsSL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz" \
    -o /tmp/SDL2.tar.gz && \
    tar -xzf /tmp/SDL2.tar.gz -C /tmp && \
    cd /tmp/SDL2-${SDL2_VERSION} && \
    ./configure --prefix=/usr --enable-video-kmsdrm --enable-video-opengles2 --disable-video-vulkan --disable-static && \
    make -j"$(nproc)" && \
    make install && \
    ldconfig && \
    rm -rf /tmp/SDL2*

WORKDIR /work
