cmake_minimum_required(VERSION 3.10)

# user binary name.
set(TARGET flutter-client)

# source files for user apps.
set(USER_APP_SRCS
  examples/flutter-wayland-client/flutter_window.cc
  examples/flutter-wayland-client/main.cc
)

# header files for user apps.
set(USER_APP_INCLUDE_DIRS
  ## Public APIs for developers (Don't edit!).
  src/client_wrapper/include
  src/flutter/shell/platform/common/client_wrapper
  src/flutter/shell/platform/common/client_wrapper/include
  src/flutter/shell/platform/common/public
  src/flutter/shell/platform/linux_embedded/public
  src/public/include
  ## header file include path for user apps.
  examples/flutter-wayland-client
)

# link libraries for user apps.
set(USER_APP_LIBRARIES "")
