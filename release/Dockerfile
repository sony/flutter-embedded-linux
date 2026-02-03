FROM debian:trixie-slim

ENV DEBIAN_FRONTEND noninteractive

RUN <<EOF
dpkg --add-architecture arm64
apt-get update
apt-get -y -q upgrade
apt-get -y install                \
       crossbuild-essential-arm64        \
       cmake                             \
       git                          \
       xxhash                       \
       curl xz-utils build-essential pkgconf \
       zip equivs clang libgles2-mesa-dev wayland-protocols libegl1-mesa-dev libdrm-dev libgbm-dev libinput-dev libudev-dev libsystemd-dev libxkbcommon-dev libwayland-dev  \
       libgles2-mesa-dev:arm64 wayland-protocols:arm64 libegl1-mesa-dev:arm64 libdrm-dev:arm64 libgbm-dev:arm64 libinput-dev:arm64 libudev-dev:arm64 libsystemd-dev:arm64 libxkbcommon-dev:arm64 libwayland-dev:arm64 libuv1-dev:arm64
apt-get clean
EOF
