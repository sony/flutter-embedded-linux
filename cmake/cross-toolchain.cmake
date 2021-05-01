cmake_minimum_required(VERSION 3.10)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the sysroot
set(sdk_target_sysroot "/opt/poky/3.1.7/sysroots/aarch64-poky-linux")
set(CMAKE_SYSROOT ${sdk_target_sysroot})

# Specify the cross compiler
set(toolchain "/opt/poky/3.1.7/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/")
set(toolchain_prefix "aarch64-poky-linux-")
set(triple aarch64-poky-linux)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_C_COMPILER ${toolchain}${toolchain_prefix}clang)
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER ${toolchain}${toolchain_prefix}clang++)

# Specify the flags
set(CFLAGS " -O2 -pipe -g -feliminate-unused-debug-types ")
set(CXXFLAGS " -O2 -pipe -g -feliminate-unused-debug-types ")
set(CMAKE_C_FLAGS ${CFLAGS} CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS ${CXXFLAGS}  CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "" FORCE)
set(CMAKE_LDFLAGS_FLAGS ${CMAKE_CXX_FLAGS} CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH ${sdk_target_sysroot})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

