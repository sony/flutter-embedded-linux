cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "video_player")
set(PLUGIN_NAME "${PROJECT_NAME}_plugin")
project(${PROJECT_NAME} LANGUAGES CXX)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED 
  gstreamer-1.0
  gstreamer-app-1.0
  gstreamer-video-1.0
  gstreamer-audio-1.0
)

add_library(${PLUGIN_NAME}
  SHARED
    "${PLUGIN_NAME}.cc"
    "gst_video_player.cc"
)

target_compile_definitions(${PLUGIN_NAME}
  PRIVATE
    FLUTTER_PLUGIN_IMPL
)

target_include_directories(${PLUGIN_NAME}
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${PLUGIN_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/flutter/shell/platform/common/client_wrapper/include
    ${CMAKE_SOURCE_DIR}/src/flutter/shell/platform/common/public
)

target_include_directories(${PLUGIN_NAME}
  PRIVATE
    ${GLIB_INCLUDE_DIR}
    ${GSTREAMER_INCLUDE_DIR}
    /usr/include/gstreamer-1.0
    /usr/include/glib-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PLUGIN_NAME}
  PRIVATE
    ${GLIB_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
)
