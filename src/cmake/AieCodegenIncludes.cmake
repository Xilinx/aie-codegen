###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

function(aie_codegen_setup_build_include_layout)
  set(_inc_root "${CMAKE_CURRENT_BINARY_DIR}/include")
  file(MAKE_DIRECTORY "${_inc_root}/aie_codegen_inc")

  # Headers are copied at build time via a cmake -P script so that GLOB runs
  # fresh on each build. Configure-time copies bake resolved paths into the
  # generated build rules; stale rules cause MSB8066 failures in CI when a
  # prior build directory is reused after a source-tree relocation.
  add_custom_target(copy_headers ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/aie_codegen.h"
      "${_inc_root}/aie_codegen.h"
    COMMAND ${CMAKE_COMMAND}
      "-DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DREGDB_DIR=${CMAKE_CURRENT_SOURCE_DIR}/../aie-regdb/globalparams"
      "-DDST_DIR=${_inc_root}/aie_codegen_inc"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyHeaders.cmake"
    COMMENT "Syncing headers to build include directory"
  )
endfunction()

function(aie_codegen_apply_include_directories target)
  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/aie_codegen_inc>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aie_codegen_inc>
  )
  add_dependencies(${target} copy_headers)
endfunction()
