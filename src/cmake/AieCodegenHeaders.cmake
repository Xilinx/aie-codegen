###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
# Header dirs for install and build-time aie_codegen_inc (keep in sync with AieCodegenSources.cmake).

set(_AIE_CODEGEN_HEADER_DIRS
  common core device dma events global interrupt
  io_backend io_backend/ext io_backend/privilege io_backend/swig
  locks memory noc npi perfcnt pl pm routing stream_switch timer trace
)

function(aie_codegen_collect_headers out_var)
  set(_headers "")
  foreach(_dir IN LISTS _AIE_CODEGEN_HEADER_DIRS)
    file(GLOB _dir_headers CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/*.h")
    list(APPEND _headers ${_dir_headers})
  endforeach()
  file(GLOB _regdb_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../aie-regdb/globalparams/*.h")
  list(APPEND _headers ${_regdb_headers})
  set(${out_var} "${_headers}" PARENT_SCOPE)
endfunction()
