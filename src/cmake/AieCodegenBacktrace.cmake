###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
#
# Backtrace-in-.DEBUG feature (Linux/GCC-Clang only, requires libdw). Mode and
# frame-capture depth are fixed at compile time via aie_codegen_apply_backtrace_options().
# Disabled by default (mode 0, depth 0) unless explicitly turned on via a
# cmake flag (-DAIE_CODEGEN_DEBUG_BACKTRACE=1/2) or the matching env var.

# Defaults a cache var from a same-named env var; an explicit -D always wins.
macro(_aie_codegen_seed_from_env var_name)
  if(NOT DEFINED ${var_name} AND DEFINED ENV{${var_name}})
    set(${var_name} "$ENV{${var_name}}")
  endif()
endmacro()

_aie_codegen_seed_from_env(AIE_CODEGEN_DEBUG_BACKTRACE)
if(NOT DEFINED AIE_CODEGEN_DEBUG_BACKTRACE)
  set(AIE_CODEGEN_DEBUG_BACKTRACE "0")
endif()
set(AIE_CODEGEN_DEBUG_BACKTRACE "${AIE_CODEGEN_DEBUG_BACKTRACE}" CACHE STRING
  "Backtrace mode: 1 = full DWARF args, 2 = function names only, 0/empty = disabled. Defaults to 0.")
if(AIE_CODEGEN_DEBUG_BACKTRACE AND NOT AIE_CODEGEN_DEBUG_BACKTRACE MATCHES "^[12]$")
  message(FATAL_ERROR "AIE_CODEGEN_DEBUG_BACKTRACE must be 1 or 2 (got: '${AIE_CODEGEN_DEBUG_BACKTRACE}')")
endif()

_aie_codegen_seed_from_env(AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH)
if(NOT DEFINED AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH)
  set(AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH "0")
endif()
set(AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH "${AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH}" CACHE STRING
  "Backtrace frame-capture depth, 0-64 (0 = no frames captured). Defaults to 0 -- set explicitly (flag or env var) to get any frames when AIE_CODEGEN_DEBUG_BACKTRACE is enabled.")
# STREQUAL "", not a bare truth test: CMake's if() treats "0" as false too.
if(NOT AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH STREQUAL "" AND
   (NOT AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH MATCHES "^[0-9]+$" OR
    AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH GREATER 64))
  message(FATAL_ERROR
    "AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH must be an integer between 0 and 64 (got: '${AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH}')")
endif()

# Links libdw and defines XAIE_BACKTRACE_ENABLED/MODE/DEPTH on `target`.
# Fails configure if AIE_CODEGEN_DEBUG_BACKTRACE is set but libdw is missing;
# otherwise a silent no-op when libdw isn't found. MODE/DEPTH are always
# defined (0/0 defaults) so xaie_controlcode.c can read them with no #ifdef.
# DEPTH is only meaningful once MODE is 1 or 2; it stays 0 whenever MODE is 0.
# Linux/GCC-Clang only; call this outside if(MSVC).
function(aie_codegen_apply_backtrace_options target)
  set(_bt_mode 0)
  set(_bt_depth 0)
  if(AIE_CODEGEN_DEBUG_BACKTRACE)
    set(_bt_mode ${AIE_CODEGEN_DEBUG_BACKTRACE})
    if(NOT AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH STREQUAL "")
      set(_bt_depth ${AIE_CODEGEN_DEBUG_BACKTRACE_DEPTH})
    endif()
  endif()

  # xaie_controlcode.c reads XAIE_BACKTRACE_MODE/DEPTH unconditionally, so
  # these must be defined before any early return below (e.g. libdw missing).
  target_compile_definitions(${target} PRIVATE
    XAIE_BACKTRACE_MODE=${_bt_mode}
    XAIE_BACKTRACE_DEPTH=${_bt_depth})

  find_library(LIBDW_LIB dw)
  if(AIE_CODEGEN_DEBUG_BACKTRACE AND NOT LIBDW_LIB)
    message(FATAL_ERROR
      "AIE_CODEGEN_DEBUG_BACKTRACE=${AIE_CODEGEN_DEBUG_BACKTRACE} was requested but libdw was not found. "
      "Install elfutils-devel (RHEL/CentOS) or libdw-dev (Debian/Ubuntu) and reconfigure.")
  endif()
  # XAIE_BACKTRACE_ENABLED (and the -g/frame-pointer/libdw linkage it implies)
  # must only be turned on when the feature was explicitly requested -- not
  # merely because libdw happens to be present on the build machine, which
  # would silently compile the whole backtrace subsystem into every build.
  if(NOT AIE_CODEGEN_DEBUG_BACKTRACE OR NOT LIBDW_LIB)
    return()
  endif()

  target_compile_options(${target} PRIVATE -g -fno-omit-frame-pointer)
  target_link_libraries(${target} PRIVATE dw pthread dl stdc++)
  target_compile_definitions(${target} PRIVATE XAIE_BACKTRACE_ENABLED)

  message(STATUS "aie_codegen: backtrace mode ${_bt_mode}, depth ${_bt_depth} compiled in (fixed at build time)")
endfunction()
