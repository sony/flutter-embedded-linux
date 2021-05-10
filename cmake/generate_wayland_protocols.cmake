cmake_minimum_required(VERSION 3.10)

function(generate_wayland_client_protocol)
  cmake_parse_arguments(ARGS "" "PROTOCOL_FILE" "CODE_FILE" "HEADER_FILE" "" ${ARGN})

  find_program(WaylandScannerExec NAMES wayland-scanner)

  get_filename_component(_xml_file ${ARGS_PROTOCOL_FILE} ABSOLUTE)
  set(_client_header ${ARGS_HEADER_FILE})
  set(_code ${ARGS_CODE_FILE})
  set_source_files_properties(${_client_header} GENERATED)
  set_source_files_properties(${_code} GENERATED)

  add_custom_command(
    OUTPUT ${_client_header}
    COMMAND ${WaylandScannerExec} client-header ${_xml_file} ${_client_header}
    DEPENDS ${_xml_file} VERBATIM
  )

  add_custom_command(
    OUTPUT ${_code}
    COMMAND ${WaylandScannerExec} private-code ${_xml_file} ${_code}
    DEPENDS ${_xml_file} ${_client_header} VERBATIM
  )
endfunction()
