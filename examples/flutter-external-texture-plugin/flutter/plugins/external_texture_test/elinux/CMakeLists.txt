cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "external_texture_test")
set(PLUGIN_NAME "${PROJECT_NAME}_plugin")
project(${PROJECT_NAME} LANGUAGES CXX)

add_library(${PLUGIN_NAME}
  SHARED
    "${PLUGIN_NAME}.cc"
)

set_target_properties(${PLUGIN_NAME}
  PROPERTIES
    CXX_VISIBILITY_PRESET hidden
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
