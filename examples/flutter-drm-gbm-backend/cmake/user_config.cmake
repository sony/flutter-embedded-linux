cmake_minimum_required(VERSION 3.10)

# Flutter embedder configurations.
# See: https://github.com/sony/flutter-embedded-linux/wiki/Building-Embedded-Linux-embedding-for-Flutter#user-configuration-parameters-cmake-options
set(BACKEND_TYPE DRM-GBM)
set(DESKTOP_SHELL OFF)
set(USE_GLES3 OFF)
