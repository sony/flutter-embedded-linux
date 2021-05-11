cmake_minimum_required(VERSION 3.10)

function(generate_wayland_client_protocol)
  cmake_parse_arguments(ARGS "" "PROTOCOL_FILE" "CODE_FILE;HEADER_FILE" ${ARGN})

  find_program(WaylandScannerExec NAMES wayland-scanner)

  get_filename_component(_xml_file ${ARGS_PROTOCOL_FILE} ABSOLUTE)
  set_source_files_properties(${ARGS_HEADER_FILE} GENERATED)
  set_source_files_properties(${ARGS_CODE_FILE} GENERATED)

  add_custom_command(
    OUTPUT ${ARGS_HEADER_FILE}
    COMMAND ${WaylandScannerExec} client-header ${_xml_file} ${ARGS_HEADER_FILE}
    DEPENDS ${_xml_file} VERBATIM
  )

  add_custom_command(
    OUTPUT ${ARGS_CODE_FILE}
    COMMAND ${WaylandScannerExec} private-code ${_xml_file} ${ARGS_CODE_FILE}
    DEPENDS ${_xml_file} ${ARGS_HEADER_FILE} VERBATIM
  )
endfunction()
