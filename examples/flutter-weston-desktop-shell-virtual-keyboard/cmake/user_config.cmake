cmake_minimum_required(VERSION 3.10)

# Flutter embedder configurations.
# See: https://github.com/sony/flutter-embedded-linux#user-configuration-parameters-cmake-options
set(BACKEND_TYPE WAYLAND)
set(DESKTOP_SHELL ON)
set(USE_VIRTUAL_KEYBOARD ON)
set(USE_GLES3 OFF)
