cmake_minimum_required(VERSION 3.10)

# display backend type.
set(DISPLAY_BACKEND_SRC "")
if(USE_DRM)
  add_definitions(-DDISPLAY_BACKEND_TYPE_DRM)
  set(DISPLAY_BACKEND_SRC
    src/flutter/shell/platform/linux_embedded/window/linuxes_window_drm.cc
    src/flutter/shell/platform/linux_embedded/surface/native_window_drm.cc)
else()
  find_program(WaylandScannerExec NAMES wayland-scanner)
  get_filename_component(_infile /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ABSOLUTE)
  set(_client_header ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland/protocol/xdg-shell-client-protocol.h)
  set(_code ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland/protocol/xdg-shell-protocol.c)
  set_source_files_properties(${_client_header} GENERATED)
  set_source_files_properties(${_code} GENERATED)
  add_custom_command(
    OUTPUT ${_client_header}
    COMMAND ${WaylandScannerExec} client-header ${_infile} ${_client_header}
    DEPENDS ${_infile} VERBATIM
  )
  add_custom_command(
    OUTPUT ${_code}
    COMMAND ${WaylandScannerExec} private-code ${_infile} ${_code}
    DEPENDS ${_infile} ${_client_header} VERBATIM
  )

  add_definitions(-DWL_EGL_PLATFORM)
  add_definitions(-DDISPLAY_BACKEND_TYPE_WAYLAND)
  set(DISPLAY_BACKEND_SRC
    ${_code}
    src/flutter/shell/platform/linux_embedded/window/linuxes_window_wayland.cc
    src/flutter/shell/platform/linux_embedded/surface/native_window_wayland.cc)
endif()

# desktop-shell for weston.
if(NOT USE_DRM AND DESKTOP_SHELL)
  add_definitions(-DDESKTOP_SHELL)
endif()

# wayland & weston protocols.
set(WAYLAND_PROTOCOL_SRC "")
if(NOT USE_DRM AND DESKTOP_SHELL)
  set(WAYLAND_PROTOCOL_SRC ${WAYLAND_PROTOCOL_SRC} src/wayland/protocol/weston-desktop-shell-protocol.c)  

  if(USE_VIRTUAL_KEYBOARD)
    add_definitions(-DUSE_VIRTUAL_KEYBOARD)
    set(WAYLAND_PROTOCOL_SRC ${WAYLAND_PROTOCOL_SRC} src/wayland/protocol/text-input-unstable-v1-protocol.c)
  endif()
endif()

# OpenGL ES version.
if(USE_GLES3)
  add_definitions(-DUSE_GLES3)
endif()

# cmake script for developers.
include(${USER_PROJECT_PATH}/cmake/user_build.cmake)

add_executable(${TARGET}
  ${USER_APP_SRCS}
  src/client_wrapper/flutter_engine.cc
  src/client_wrapper/flutter_view_controller.cc
  src/flutter/shell/platform/linux_embedded/flutter_linuxes.cc
  src/flutter/shell/platform/linux_embedded/flutter_linuxes_engine.cc
  src/flutter/shell/platform/linux_embedded/flutter_linuxes_view.cc
  src/flutter/shell/platform/linux_embedded/flutter_project_bundle.cc
  src/flutter/shell/platform/linux_embedded/task_runner.cc
  src/flutter/shell/platform/linux_embedded/system_utils.cc
  src/flutter/shell/platform/linux_embedded/logger.cc
  src/flutter/shell/platform/linux_embedded/external_texture_gl.cc
  src/flutter/shell/platform/linux_embedded/flutter_linuxes_texture_registrar.cc
  src/flutter/shell/platform/linux_embedded/plugin/key_event_plugin.cc
  src/flutter/shell/platform/linux_embedded/plugin/key_event_plugin_glfw_util.cc
  src/flutter/shell/platform/linux_embedded/plugin/text_input_plugin.cc
  src/flutter/shell/platform/linux_embedded/plugin/platform_plugin.cc
  src/flutter/shell/platform/linux_embedded/plugin/mouse_cursor_plugin.cc
  src/flutter/shell/platform/linux_embedded/surface/egl_utils.cc
  ${DISPLAY_BACKEND_SRC}
  ${WAYLAND_PROTOCOL_SRC}
  ## The following file were copied from:
  ## https://github.com/flutter/engine/blob/master/shell/platform/glfw/
  src/flutter/shell/platform/linux_embedded/system_utils.cc
  ## Following files were imported from:
  ## https://github.com/flutter/engine/tree/master/shell/platform/common
  src/flutter/shell/platform/common/client_wrapper/engine_method_result.cc
  src/flutter/shell/platform/common/client_wrapper/standard_codec.cc
  src/flutter/shell/platform/common/client_wrapper/plugin_registrar.cc
  src/flutter/shell/platform/common/text_input_model.cc
  src/flutter/shell/platform/common/json_message_codec.cc
  src/flutter/shell/platform/common/json_method_codec.cc
  src/flutter/shell/platform/common/engine_switches.cc
  src/flutter/shell/platform/common/incoming_message_dispatcher.cc
)

set(THIRD_PARTY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party)
target_include_directories(${TARGET}
  PRIVATE
    src
    ## third-party libraries.
    ${XKBCOMMON_INCLUDE_DIRS}
    ${THIRD_PARTY_INCLUDE_DIR}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${WAYLAND_CURSOR_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLES_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIRS}
    ${GBM_INCLUDE_DIRS}
    ${LIBINPUT_INCLUDE_DIRS}
    ${LIBUDEV_INCLUDE_DIRS}
    ${LIBSYSTEMD_INCLUDE_DIRS}
    ${LIBWESTON_INCLUDE_DIRS}
    ## User libraries
    ${USER_APP_INCLUDE_DIRS}
)

set(CMAKE_SKIP_RPATH true)
set(FLUTTER_EMBEDDER_LIB /usr/lib/libflutter_engine.so)
target_link_libraries(${TARGET}
  PRIVATE
    ${XKBCOMMON_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_CURSOR_LIBRARIES}
    ${WAYLAND_EGL_LIBRARIES}
    ${EGL_LIBRARIES}
    ${DRM_LIBRARIES}
    ${GBM_LIBRARIES}
    ${LIBINPUT_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    ${LIBSYSTEMD_LIBRARIES}
    ${LIBWESTON_LIBRARIES}
    ${FLUTTER_EMBEDDER_LIB}
    ## User libraries
    ${USER_APP_LIBRARIES}
)

if(USE_DRM)
target_link_libraries(${TARGET}
    PRIVATE
      Threads::Threads
)
endif()

target_compile_options(${TARGET}
  PUBLIC
    ${EGL_CFLAGS}
)
