###############################################################################
# Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
# Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

# FindLibaie_codegen
# --------
#
# Find libaie_codegen
#
# Find the native libaie_codegen includes and library this module defines
#
# ::
#
#   LIBAIE_CODEGEN_INCLUDE_DIR, where to find aie_codegen.h, etc.

find_path(LIBAIE_CODEGEN_INCLUDE_DIR NAMES aie_codegen.h PATHS ${CMAKE_FIND_ROOT_PATH})
find_library(LIBAIE_CODEGEN_LIB NAMES aie_codegen PATHS ${CMAKE_FIND_ROOT_PATH})
get_filename_component(LIBAIE_CODEGEN_LIB_DIR ${LIBAIE_CODEGEN_LIB} DIRECTORY)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (LIBAIE_CODEGEN DEFAULT_MSG LIBAIE_CODEGEN_LIB LIBAIE_CODEGEN_INCLUDE_DIR)

mark_as_advanced (LIBAIE_CODEGEN_LIB LIBAIE_CODEGEN_INCLUDE_DIR LIBAIE_CODEGEN_LIB_DIR)
