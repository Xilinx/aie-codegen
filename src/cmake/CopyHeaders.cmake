###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
# Build-time script: invoked via cmake -P by the copy_headers custom target.
# Runs GLOB at build time so that paths are resolved fresh on each build rather
# than being baked into generated build rules at configure time.
#
# Expected variables (passed via -D on the cmake -P command line):
#   SRC_DIR   - aie-codegen/src source directory
#   REGDB_DIR - aie-regdb/globalparams source directory
#   DST_DIR   - destination include/aie_codegen_inc directory

foreach(_var SRC_DIR REGDB_DIR DST_DIR)
  if(NOT DEFINED ${_var})
    message(FATAL_ERROR "CopyHeaders.cmake: required variable ${_var} is not defined")
  endif()
endforeach()

file(GLOB_RECURSE _hdrs
  "${SRC_DIR}/*/*.h"
  "${SRC_DIR}/*/*/*.h"
  "${REGDB_DIR}/*.h"
)

foreach(_hdr IN LISTS _hdrs)
  get_filename_component(_name "${_hdr}" NAME)
  file(COPY_FILE "${_hdr}" "${DST_DIR}/${_name}" ONLY_IF_DIFFERENT)
endforeach()
