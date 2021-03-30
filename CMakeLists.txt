cmake_minimum_required(VERSION 3.10)
project("flutter_linuxes" LANGUAGES CXX C)

# Rules of compiler and cpp version.
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)

# Build options.
option(USE_DRM "Select WAYLAND or DRM as the display backend type" OFF)
option(USE_EGLSTREAM "Select WAYLAND or EGLStream as the display backend type" OFF)
option(DESKTOP_SHELL "Work as weston desktop-shell" OFF)
option(USE_VIRTUAL_KEYBOARD "Use virtual keyboard" OFF)
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
