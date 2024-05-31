cmake_minimum_required(VERSION 3.10)

# The platform-dependent definitions such as EGLNativeDisplayType and 
# EGLNativeWindowType depend on related include files or define such as gbm.h
# or "__GBM__". So, need to avoid a link error which is caused by the 
# include order of related header files. See: /usr/include/EGL/eglplatform.h
if(${BACKEND_TYPE} STREQUAL "DRM-GBM")
  add_definitions(-D__GBM__)
elseif(${BACKEND_TYPE} STREQUAL "DRM-EGLSTREAM")
  add_definitions(-DEGL_NO_X11)
elseif(${BACKEND_TYPE} STREQUAL "X11")
  add_definitions(-DUSE_X11)
else()
  add_definitions(-DWL_EGL_PLATFORM)
endif()

# display backend type.
set(DISPLAY_BACKEND_SRC "")
if(${BACKEND_TYPE} STREQUAL "DRM-GBM")
  add_definitions(-DDISPLAY_BACKEND_TYPE_DRM_GBM)
  add_definitions(-DFLUTTER_TARGET_BACKEND_GBM)
  set(DISPLAY_BACKEND_SRC
    "src/flutter/shell/platform/linux_embedded/window/native_window_drm.cc"
    "src/flutter/shell/platform/linux_embedded/window/native_window_drm_gbm.cc")
elseif(${BACKEND_TYPE} STREQUAL "DRM-EGLSTREAM")
  add_definitions(-DDISPLAY_BACKEND_TYPE_DRM_EGLSTREAM)
  add_definitions(-DFLUTTER_TARGET_BACKEND_EGLSTREAM)
  set(DISPLAY_BACKEND_SRC
    "src/flutter/shell/platform/linux_embedded/surface/context_egl_stream.cc"
    "src/flutter/shell/platform/linux_embedded/surface/environment_egl_stream.cc"
    "src/flutter/shell/platform/linux_embedded/window/native_window_drm.cc"
    "src/flutter/shell/platform/linux_embedded/window/native_window_drm_eglstream.cc")
elseif(${BACKEND_TYPE} STREQUAL "X11")
  add_definitions(-DDISPLAY_BACKEND_TYPE_X11)
  add_definitions(-DFLUTTER_TARGET_BACKEND_X11)
  set(DISPLAY_BACKEND_SRC
    "src/flutter/shell/platform/linux_embedded/window/elinux_window_x11.cc"
    "src/flutter/shell/platform/linux_embedded/window/native_window_x11.cc")
else()
  include(cmake/generate_wayland_protocols.cmake)
  pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
  set(_wayland_protocols_xml_dir "${WAYLAND_PROTOCOLS_DATADIR}")
  set(_wayland_protocols_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/wayland/protocols")
  file(MAKE_DIRECTORY "${_wayland_protocols_src_dir}")

  generate_wayland_client_protocol(
    PROTOCOL_FILE "${_wayland_protocols_xml_dir}/stable/xdg-shell/xdg-shell.xml"
    CODE_FILE "${_wayland_protocols_src_dir}/xdg-shell-protocol.c"
    HEADER_FILE "${_wayland_protocols_src_dir}/xdg-shell-client-protocol.h")

  generate_wayland_client_protocol(
    PROTOCOL_FILE "${_wayland_protocols_xml_dir}/unstable/text-input/text-input-unstable-v1.xml"
    CODE_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v1-protocol.c"
    HEADER_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v1-client-protocol.h")

  generate_wayland_client_protocol(
    PROTOCOL_FILE "${_wayland_protocols_xml_dir}/unstable/text-input/text-input-unstable-v3.xml"
    CODE_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v3-protocol.c"
    HEADER_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v3-client-protocol.h")

  generate_wayland_client_protocol(
    PROTOCOL_FILE "${_wayland_protocols_xml_dir}/stable/presentation-time/presentation-time.xml"
    CODE_FILE "${_wayland_protocols_src_dir}/presentation-time-protocol.c"
    HEADER_FILE "${_wayland_protocols_src_dir}/presentation-time-protocol.h")

  generate_wayland_client_protocol(
    PROTOCOL_FILE "${_wayland_protocols_xml_dir}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
    CODE_FILE "${_wayland_protocols_src_dir}/xdg-decoration-unstable-v1-protocol.c"
    HEADER_FILE "${_wayland_protocols_src_dir}/xdg-decoration-unstable-v1-protocol.h")

  add_definitions(-DFLUTTER_TARGET_BACKEND_WAYLAND)
  add_definitions(-DDISPLAY_BACKEND_TYPE_WAYLAND)
  set(DISPLAY_BACKEND_SRC
    "${_wayland_protocols_src_dir}/xdg-shell-protocol.c"
    "${_wayland_protocols_src_dir}/text-input-unstable-v1-protocol.c"
    "${_wayland_protocols_src_dir}/text-input-unstable-v3-protocol.c"
    "${_wayland_protocols_src_dir}/presentation-time-protocol.c"
    "${_wayland_protocols_src_dir}/xdg-decoration-unstable-v1-protocol.c"
    "src/flutter/shell/platform/linux_embedded/window/elinux_window_wayland.cc"
    "src/flutter/shell/platform/linux_embedded/window/native_window_wayland.cc"
    "src/flutter/shell/platform/linux_embedded/window/native_window_wayland_decoration.cc"
    "src/flutter/shell/platform/linux_embedded/window/renderer/elinux_shader.cc"
    "src/flutter/shell/platform/linux_embedded/window/renderer/elinux_shader_context.cc"
    "src/flutter/shell/platform/linux_embedded/window/renderer/elinux_shader_program.cc"
    "src/flutter/shell/platform/linux_embedded/window/renderer/window_decoration_button.cc"
    "src/flutter/shell/platform/linux_embedded/window/renderer/window_decoration_titlebar.cc"
    "src/flutter/shell/platform/linux_embedded/window/renderer/window_decorations_wayland.cc")
endif()

# Use flutter dirty region management
if(USE_DIRTY_REGION_MANAGEMENT)
  add_definitions(-DUSE_OPENGL_DIRTY_REGION_MANAGEMENT)
endif()

# OpenGL ES version.
if(USE_GLES3)
  add_definitions(-DUSE_GLES3)
endif()

# Flutter embedder runtime mode.
if(FLUTTER_RELEASE)
  add_definitions(
    -DFLUTTER_RELEASE # release mode
  )
endif()

# Enable logger of eLinux embedder.
if(ENABLE_ELINUX_EMBEDDER_LOG)
  add_definitions(
    -DENABLE_ELINUX_EMBEDDER_LOG
  )
endif()

# Enable embedder vsync.
if(ENABLE_VSYNC)
  add_definitions(
    -DENABLE_VSYNC
  )
endif()

# Enable alpha component of the egl color buffer.
if(ENABLE_EGL_ALPHA_COMPONENT_OF_COLOR_BUFFER)
  add_definitions(
    -DENABLE_EGL_ALPHA_COMPONENT_OF_COLOR_BUFFER
  )
endif()

set(CPP_WRAPPER_SOURCES_CORE
  "src/flutter/shell/platform/common/client_wrapper/engine_method_result.cc"
  "src/flutter/shell/platform/common/client_wrapper/standard_codec.cc"
  "src/flutter/shell/platform/common/client_wrapper/string_message_codec.cc"
)
set(CPP_WRAPPER_SOURCES_PLUGIN
  "src/flutter/shell/platform/common/client_wrapper/plugin_registrar.cc"
)
set(CPP_WRAPPER_SOURCES_APP
  "src/client_wrapper/flutter_engine.cc"
  "src/client_wrapper/flutter_view_controller.cc"
)

set(ELINUX_COMMON_SRC
  "src/flutter/shell/platform/linux_embedded/flutter_elinux.cc"
  "src/flutter/shell/platform/linux_embedded/flutter_elinux_engine.cc"
  "src/flutter/shell/platform/linux_embedded/flutter_elinux_view.cc"
  "src/flutter/shell/platform/linux_embedded/flutter_project_bundle.cc"
  "src/flutter/shell/platform/linux_embedded/task_runner.cc"
  "src/flutter/shell/platform/linux_embedded/system_utils.cc"
  "src/flutter/shell/platform/linux_embedded/logger.cc"
  "src/flutter/shell/platform/linux_embedded/external_texture_pixelbuffer.cc"
  "src/flutter/shell/platform/linux_embedded/external_texture_egl_image.cc"
  "src/flutter/shell/platform/linux_embedded/vsync_waiter.cc"
  "src/flutter/shell/platform/linux_embedded/flutter_elinux_texture_registrar.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/keyboard_glfw_util.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/key_event_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/lifecycle_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/mouse_cursor_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/navigation_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/platform_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/platform_views_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/settings_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/plugins/text_input_plugin.cc"
  "src/flutter/shell/platform/linux_embedded/surface/context_egl.cc"
  "src/flutter/shell/platform/linux_embedded/surface/egl_utils.cc"
  "src/flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.cc"
  "src/flutter/shell/platform/linux_embedded/surface/surface_base.cc"
  "src/flutter/shell/platform/linux_embedded/surface/surface_gl.cc"
  "src/flutter/shell/platform/linux_embedded/surface/surface_decoration.cc"
  "${DISPLAY_BACKEND_SRC}"
  ## The following file were copied from:
  ## https://github.com/flutter/engine/blob/master/shell/platform/glfw/
  "src/flutter/shell/platform/linux_embedded/system_utils.cc"
  ## Following files were imported from:
  ## https://github.com/flutter/engine/tree/master/shell/platform/common
  "src/flutter/shell/platform/common/text_input_model.cc"
  "src/flutter/shell/platform/common/json_message_codec.cc"
  "src/flutter/shell/platform/common/json_method_codec.cc"
  "src/flutter/shell/platform/common/engine_switches.cc"
  "src/flutter/shell/platform/common/incoming_message_dispatcher.cc"
)

if(NOT BUILD_ELINUX_SO)
  # cmake script for developers.
  include(${USER_PROJECT_PATH}/cmake/user_build.cmake)

  add_executable(${TARGET}
    ${ELINUX_COMMON_SRC}
    ${CPP_WRAPPER_SOURCES_CORE}
    ${CPP_WRAPPER_SOURCES_PLUGIN}
    ${CPP_WRAPPER_SOURCES_APP}
    ${USER_APP_SRCS}
  )
else()
  add_library(${TARGET}
    SHARED
      ${ELINUX_COMMON_SRC}
      ${CPP_WRAPPER_SOURCES_CORE}
      ${CPP_WRAPPER_SOURCES_PLUGIN}
  )

  target_include_directories(${TARGET}
    PUBLIC
      "src/flutter/shell/platform/linux_embedded/public"
  )

  target_include_directories(${TARGET}
    PRIVATE
      "src/client_wrapper/include"
      "src/flutter/shell/platform/common/client_wrapper"
      "src/flutter/shell/platform/common/client_wrapper/include"
      "src/flutter/shell/platform/common/client_wrapper/include/flutter"
      "src/flutter/shell/platform/common/public"
  )
endif()

set(THIRD_PARTY_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party")
set(RAPIDJSON_INCLUDE_DIRS "${THIRD_PARTY_DIRS}/rapidjson/include/")
target_include_directories(${TARGET}
  PRIVATE
    "src"
    ## third-party libraries.
    ${THIRD_PARTY_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
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
    ${X11_INCLUDE_DIRS}
    ${LIBWESTON_INCLUDE_DIRS}
    ## User libraries
    ${USER_APP_INCLUDE_DIRS}
)

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
    ${LIBUV_LIBRARIES}
    ${X11_LIBRARIES}
    ${LIBWESTON_LIBRARIES}
    ${FLUTTER_EMBEDDER_LIB}
    ## User libraries
    ${USER_APP_LIBRARIES}
)

if(${BACKEND_TYPE} MATCHES "^DRM-(GBM|EGLSTREAM)$")
  target_link_libraries(${TARGET}
    PRIVATE
      Threads::Threads
  )

  # Indicate whether libsystemd must replace libuv
  if(${LIBSYSTEMD_FOUND} EQUAL 1)
    add_definitions(-DUSE_LIBSYSTEMD)
  endif()
endif()

set(FLUTTER_EMBEDDER_LIB "${CMAKE_CURRENT_SOURCE_DIR}/build/libflutter_engine.so")
set(CMAKE_SKIP_RPATH true)
target_link_libraries(${TARGET}
  PRIVATE
    ${FLUTTER_EMBEDDER_LIB}
)

target_compile_options(${TARGET}
  PUBLIC
    ${EGL_CFLAGS}
)

if(NOT BUILD_ELINUX_SO)
  # Generated plugin build rules
  include(${USER_PROJECT_PATH}/flutter/generated_plugins.cmake)
endif()
