###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

get_filename_component(_AIE_CODEGEN_SOURCELINK_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

function(aie_codegen_enable_sourcelink target)
  find_package(Git QUIET)
  if(NOT GIT_FOUND)
    message(STATUS "aie_codegen: Git not found; SourceLink disabled")
    return()
  endif()

  set(_sourcelink_json "${CMAKE_CURRENT_BINARY_DIR}/aie_codegen.sourcelink.json")

  set(_git_sha_override "")
  if(DEFINED ENV{BUILD_SOURCEVERSION} AND NOT "$ENV{BUILD_SOURCEVERSION}" STREQUAL "")
    set(_git_sha_override "$ENV{BUILD_SOURCEVERSION}")
  elseif(DEFINED ENV{GIT_COMMIT} AND NOT "$ENV{GIT_COMMIT}" STREQUAL "")
    set(_git_sha_override "$ENV{GIT_COMMIT}")
  elseif(DEFINED ENV{GITHUB_SHA} AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
    set(_git_sha_override "$ENV{GITHUB_SHA}")
  endif()

  add_custom_command(
    OUTPUT "${_sourcelink_json}"
    COMMAND "${CMAKE_COMMAND}"
      -DOUT_FILE=${_sourcelink_json}
      -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
      -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
      -DGIT_SHA_OVERRIDE=${_git_sha_override}
      -P "${_AIE_CODEGEN_SOURCELINK_DIR}/GenerateSourceLink.cmake"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating Source Link file for aie_codegen"
    VERBATIM
  )

  add_custom_target(aie_codegen_sourcelink DEPENDS "${_sourcelink_json}")
  add_dependencies(${target} aie_codegen_sourcelink)
  target_link_options(${target} PRIVATE "/SOURCELINK:${_sourcelink_json}")
endfunction()
