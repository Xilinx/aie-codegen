###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

get_filename_component(_AIE_CODEGEN_MSVC_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

function(aie_codegen_apply_msvc_options target)
  set(_msvc_compile_opts
    /Wall
    /sdl
    /GS
    /GF
    /guard:cf
    /Qspectre
    /permissive-
    /wd4820
    /wd4206
    /wd5045
    /wd4668
    /wd4242
    /wd4061
    /wd5105
    /wd4710
    /wd4711
    /wd4324
    /wd4255
  )
  if(AIE_CODEGEN_ENABLE_WERROR)
    list(APPEND _msvc_compile_opts /WX)
  endif()
  target_compile_options(${target} PRIVATE ${_msvc_compile_opts})

  set_target_properties(${target} PROPERTIES
    VS_GLOBAL_SpectreMitigation "Spectre"
    LINK_INCREMENTAL_RELEASE OFF
    LINK_INCREMENTAL_RELWITHDEBINFO OFF
    LINK_INCREMENTAL_MINSIZEREL OFF
  )

  set(_msvc_link
    /DYNAMICBASE
    /guard:cf
    /LARGEADDRESSAWARE
    /DEBUG
    /OPT:REF
    /INCREMENTAL:NO
  )
  string(TOUPPER "${CMAKE_GENERATOR_PLATFORM}" _vs_plat)
  if(NOT _vs_plat STREQUAL "ARM64")
    list(APPEND _msvc_link /CETCOMPAT)
  endif()
  target_link_options(${target} PRIVATE ${_msvc_link})

  target_link_options(${target} PRIVATE
    "$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:/OPT:ICF>"
    "$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:/LTCG>"
  )

  if(AIE_CODEGEN_MSVC_RELEASE_PDB)
    target_compile_options(${target} PRIVATE /Zi)
  endif()

  if(AIE_CODEGEN_ENABLE_SOURCELINK)
    include(${_AIE_CODEGEN_MSVC_DIR}/AieCodegenSourceLink.cmake)
    aie_codegen_enable_sourcelink(${target})
  endif()
endfunction()
