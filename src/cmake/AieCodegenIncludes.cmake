###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

include(${CMAKE_CURRENT_LIST_DIR}/AieCodegenHeaders.cmake)

function(_aie_codegen_materialize_header source dest)
  file(CREATE_LINK "${source}" "${dest}" SYMBOLIC RESULT _link_result)
  if(NOT _link_result EQUAL 0)
    configure_file("${source}" "${dest}" COPYONLY)
  endif()
endfunction()

function(aie_codegen_setup_build_include_layout)
  set(_inc_root "${CMAKE_CURRENT_BINARY_DIR}/include")
  file(MAKE_DIRECTORY "${_inc_root}/aie_codegen_inc")

  aie_codegen_collect_headers(_headers)
  foreach(_hdr IN LISTS _headers)
    get_filename_component(_name "${_hdr}" NAME)
    _aie_codegen_materialize_header("${_hdr}" "${_inc_root}/aie_codegen_inc/${_name}")
  endforeach()

  _aie_codegen_materialize_header(
    "${CMAKE_CURRENT_SOURCE_DIR}/aie_codegen.h"
    "${_inc_root}/aie_codegen.h")
endfunction()

function(aie_codegen_apply_include_directories target)
  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/aie_codegen_inc>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aie_codegen_inc>
  )
endfunction()
