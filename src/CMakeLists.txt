###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

cmake_minimum_required(VERSION 3.21)
project(aie_codegen LANGUAGES C VERSION 3.5)

# --- Guards / global settings -------------------------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported. Use a separate build directory, e.g.\n"
    "  cmake -S ${CMAKE_CURRENT_SOURCE_DIR} -B build")
endif()

if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# --- Modules --------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(AieCodegenSources)
include(AieCodegenIncludes)
include(AieCodegenMsvc)

# --- Options (AIE_CODEGEN_* prefix) -------------------------------------------

option(AIE_CODEGEN_BUILD_SHARED "Build aie_codegen as a shared library" ON)
option(AIE_CODEGEN_ENABLE_ELF "Enable ELF loader functions (XAIE_FEATURE_ELF_ENABLE)" OFF)

option(CONTROLCODE_BACKEND "Enable control code IO backend" ON)
option(SOCKET_BACKEND "Enable socket IO backend" OFF)
option(DEBUG_BACKEND "Enable debug IO backend" OFF)

option(AIE_CODEGEN_ENABLE_WERROR "Treat compiler warnings as errors" ON)

if(MSVC)
  option(AIE_CODEGEN_MSVC_RELEASE_PDB
    "MSVC: /Zi program database for BinSkim / compliance tooling (all configs)" ON)
  option(AIE_CODEGEN_ENABLE_SOURCELINK
    "Embed Source Link in PDB via /SOURCELINK (BinSkim BA2027)" ON)
endif()

# --- Library ------------------------------------------------------------------

aie_codegen_setup_build_include_layout()

# Target
if(AIE_CODEGEN_BUILD_SHARED)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()
add_library(aie_codegen::aie_codegen ALIAS ${PROJECT_NAME})

# Sources
aie_codegen_add_sources(${PROJECT_NAME})

# Properties
set(_aie_codegen_target_props
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)
if(WIN32)
  list(APPEND _aie_codegen_target_props WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES ${_aie_codegen_target_props})

# Includes
aie_codegen_apply_include_directories(${PROJECT_NAME})

# Compile definitions
if(AIE_CODEGEN_ENABLE_ELF)
  target_compile_definitions(${PROJECT_NAME} PRIVATE XAIE_FEATURE_ELF_ENABLE)
  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
endif()
if(CONTROLCODE_BACKEND)
  target_compile_definitions(${PROJECT_NAME} PRIVATE __AIECONTROLCODE__)
endif()
if(SOCKET_BACKEND)
  target_compile_definitions(${PROJECT_NAME} PRIVATE __AIESOCKET__)
endif()
if(DEBUG_BACKEND)
  target_compile_definitions(${PROJECT_NAME} PRIVATE __AIEDEBUG__)
endif()
if(MSVC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE XAIE_FEATURE_MSVC)
endif()

# Compile options
if(MSVC)
  aie_codegen_apply_msvc_options(${PROJECT_NAME})
else()
  set(_gcc_warnings -Wall -Wextra)
  if(AIE_CODEGEN_ENABLE_WERROR)
    list(APPEND _gcc_warnings -Werror)
  endif()
  target_compile_options(${PROJECT_NAME} PRIVATE ${_gcc_warnings})
endif()

# --- Install / package / quality ----------------------------------------------

include(AieCodegenInstall)
aie_codegen_install(${PROJECT_NAME})

include(AieCodegenCodeQL)
