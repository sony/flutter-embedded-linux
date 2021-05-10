cmake_minimum_required(VERSION 3.10)

function(generate_wayland_client_protocol RESULT)
  cmake_parse_arguments(ARGS "" "PROTOCOL_NAME" "PROTOCOL_FILE" "" ${ARGN})

  find_program(WaylandScannerExec NAMES wayland-scanner)
  set(_wayland_protocols_dir $ENV{PKG_CONFIG_SYSROOT_DIR}/usr/share/wayland-protocols)

  get_filename_component(_infile ${_wayland_protocols_dir}/${ARGS_PROTOCOL_FILE} ABSOLUTE)
  set(_client_header ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland/protocol/${ARGS_PROTOCOL_NAME}-client-protocol.h)
  set(_code ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland/protocol/${ARGS_PROTOCOL_NAME}-protocol.c)
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

  list(APPEND ${RESULT} ${_code})
  set (${RESULT} ${${RESULT}} PARENT_SCOPE)
endfunction()
