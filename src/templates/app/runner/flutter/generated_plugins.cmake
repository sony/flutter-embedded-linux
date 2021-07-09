#
# Generated file, do not edit.
#

list(APPEND FLUTTER_PLUGIN_LIST
)

set(PLUGIN_BUNDLED_LIBRARIES)

foreach(plugin ${FLUTTER_PLUGIN_LIST})
  add_subdirectory(
    ${USER_PROJECT_PATH}/flutter/plugins/${plugin}/elinux plugins/${plugin})

  target_link_libraries(${TARGET}
    PRIVATE
      ${plugin}_plugin
  )

  list(APPEND PLUGIN_BUNDLED_LIBRARIES 
    ${PROJECT_BINARY_DIR}/plugins/${plugin}/lib${plugin}_plugin.so
  )
endforeach(plugin)
