cmake_minimum_required(VERSION 3.10)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the sysroot. You need to modify <path_to_user_target_sysroot> appropriately for your environment
set(target_sysroot "<path_to_user_target_sysroot>")
# Specify the cross compiler. You need to modify <path_to_user_toolchain> appropriately for your environment
set(toolchain "<path_to_user_toolchain>")

set(CMAKE_SYSROOT ${target_sysroot})
set(toolchain_prefix "aarch64-poky-linux-")
set(triple aarch64-poky-linux)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_C_COMPILER ${toolchain}${toolchain_prefix}clang)
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER ${toolchain}${toolchain_prefix}clang++)

set(CMAKE_FIND_ROOT_PATH ${target_sysroot})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

