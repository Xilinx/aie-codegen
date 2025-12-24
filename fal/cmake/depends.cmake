###############################################################################
# Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
# Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

if (WITH_XAIEDRV_FIND)
  find_package (Libaie_codegen REQUIRED)
  collect (PROJECT_INC_DIRS "${LIBAIE_CODEGEN_INCLUDE_DIR}")
  collect (PROJECT_LIB_DIRS "${LIBAIE_CODEGEN_LIB_DIR}")
  collect (PROJECT_LIB_DEPS "${LIBAIE_CODEGEN_LIB}")
else()
  collect (PROJECT_INC_DIRS "${CMAKE_BINARY_DIR}/driver-src/include")
  collect (PROJECT_LIB_DIRS "${CMAKE_BINARY_DIR}/driver-src/src")
  collect (PROJECT_LIB_DEPS "aie_codegen")
endif (WITH_XAIEDRV_FIND)
