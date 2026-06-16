###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

include(CMakePackageConfigHelpers)
include(${CMAKE_CURRENT_LIST_DIR}/AieCodegenHeaders.cmake)

function(aie_codegen_install target)
  aie_codegen_collect_headers(_aie_codegen_hdrs)

  install(
    FILES ${_aie_codegen_hdrs}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aie_codegen_inc
  )
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/aie_codegen.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  file(GLOB _xaiefal_common  "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/common/*")
  file(GLOB _xaiefal_profile "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/profile/*")
  file(GLOB _xaiefal_rsc     "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/rsc/*")
  file(GLOB _xaiefal_top     "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/*.hpp")

  install(FILES ${_xaiefal_common}  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xaiefal/common)
  install(FILES ${_xaiefal_profile} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xaiefal/profile)
  install(FILES ${_xaiefal_rsc}     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xaiefal/rsc)
  install(FILES ${_xaiefal_top}     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(
    TARGETS ${target}
    EXPORT ${PROJECT_NAME}-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  install(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE ${PROJECT_NAME}::
    COMPONENT runtime
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake @ONLY
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
  )
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    COMPONENT runtime
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )
endfunction()
