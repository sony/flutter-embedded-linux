cmake_minimum_required(VERSION 3.10)

# Flutter embedder configurations.
# See: https://github.com/sony/flutter-embedded-linux#user-configuration-parameters-cmake-options
set(USE_DRM ON)
set(USE_X11 OFF)
set(DESKTOP_SHELL OFF)
set(USE_VIRTUAL_KEYBOARD OFF)
set(USE_GLES3 OFF)
