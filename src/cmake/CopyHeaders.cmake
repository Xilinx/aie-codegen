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

foreach(_dir SRC_DIR REGDB_DIR)
  if(NOT IS_DIRECTORY "${${_dir}}")
    message(WARNING "CopyHeaders.cmake: ${_dir}=\"${${_dir}}\" is not a directory; "
                    "headers from this location will be missing. "
                    "Is the submodule initialized?")
  endif()
endforeach()

# Reuse the canonical header-dir list from AieCodegenHeaders.cmake so that
# build-time copies and install-time copies always cover exactly the same set.
include("${CMAKE_CURRENT_LIST_DIR}/AieCodegenHeaders.cmake")

set(_hdrs "")
foreach(_dir IN LISTS _AIE_CODEGEN_HEADER_DIRS)
  file(GLOB _dir_hdrs "${SRC_DIR}/${_dir}/*.h")
  list(APPEND _hdrs ${_dir_hdrs})
endforeach()
file(GLOB _regdb_hdrs "${REGDB_DIR}/*.h")
list(APPEND _hdrs ${_regdb_hdrs})

foreach(_hdr IN LISTS _hdrs)
  get_filename_component(_name "${_hdr}" NAME)
  file(COPY_FILE "${_hdr}" "${DST_DIR}/${_name}" ONLY_IF_DIFFERENT)
endforeach()
