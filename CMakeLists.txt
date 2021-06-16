cmake_minimum_required(VERSION 3.10)
project("flutter_elinux" LANGUAGES CXX C)

# We only support clang, so the build will be faild if you try to build with gcc compiler instead of it.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
endif()
set(CMAKE_CXX_STANDARD 17)

# Build options.
option(BACKEND_TYPE "Select WAYLAND, DRM-GBM, DRM-EGLSTREAM, or X11 as the display backend type" WAYLAND)
option(DESKTOP_SHELL "Work as weston desktop-shell" OFF)
option(USE_GLES3 "Use OpenGL ES3 (default is OpenGL ES2)" OFF)

# Load the user project.
set(USER_PROJECT_PATH "examples/flutter-wayland-client" CACHE STRING "")
message("User project: ${USER_PROJECT_PATH}")
include(${USER_PROJECT_PATH}/cmake/user_config.cmake)

# System level dependent library.
include(cmake/package.cmake)

# Rules of 3rd party libraries.
# RapidJson library.
include(cmake/rapidjson.cmake)

# Build for target.
include(cmake/build.cmake)
