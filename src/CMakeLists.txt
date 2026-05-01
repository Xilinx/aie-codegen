###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

cmake_minimum_required(VERSION 3.16.3)
project(aie_codegen LANGUAGES C CXX VERSION 3.5)

set(CMAKE_C_STANDARD 11)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# Define a custom option to control whether the library is built as a shared or static library
option(AIE_CODEGEN_BUILD_SHARED "Build aie_codegen as a shared library" ON)

file(
  GLOB_RECURSE
  SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.c"
  )

file(
  GLOB_RECURSE
  HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../aie-regdb/globalparams/*.h"
  )

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/aie_codegen_inc)

add_custom_target(copy_headers ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/aie_codegen.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/include/aie_codegen_inc/
  COMMENT "Syncing headers to build include directory"
)

if(AIE_CODEGEN_BUILD_SHARED)
  # Build the library as a shared library
  add_library(
    ${PROJECT_NAME}
    SHARED
    ${SRCS}
  )
else()
  # Build the library as a static library
  add_library(
    ${PROJECT_NAME}
    STATIC
    ${SRCS}
  )
endif()

add_dependencies(${PROJECT_NAME} copy_headers)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/aie_codegen_inc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../aie-regdb/globalparams>
  $<INSTALL_INTERFACE:include>
  $<INSTALL_INTERFACE:include/aie_codegen_inc>
  )
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  VERSION
  ${PROJECT_VERSION}
  SOVERSION
  ${PROJECT_VERSION_MAJOR}
  )

if (SOCKET_BACKEND)
	list(APPEND IO_BACKEND -D__AIESOCKET__)
endif (SOCKET_BACKEND)
if (DEBUG_BACKEND)
	list(APPEND IO_BACKEND -D__AIEDEBUG__)
endif (DEBUG_BACKEND)

#By default, CERT control code backend will be 
#enabled while compiling aie-rt using this cmake
list(APPEND IO_BACKEND -D__AIECONTROLCODE__)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
   /wd4018 /wd4024 /wd4047 /wd4061 /wd4100 /wd4200 /wd4206 /wd4242 /wd4244 /wd4267 /wd4388 /wd4389 /wd4431 /wd4668 /wd4701 /wd4710 /wd4711 /wd4716
   /wd4820 /wd4996 /wd5045 /wd5105
   )
  target_compile_options(${PROJECT_NAME} PRIVATE /Wall -DXAIE_FEATURE_MSVC ${IO_BACKEND})
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra ${IO_BACKEND})
endif()

include(GNUInstallDirs)

install(
  FILES
  ${HDRS}
  DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}/aie_codegen_inc
  )

install(
  FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/aie_codegen.h
  DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}
  )

# FAL (xaiefal) header-only C++ wrappers
file(GLOB _XAIEFAL_COMMON  "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/common/*")
file(GLOB _XAIEFAL_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/profile/*")
file(GLOB _XAIEFAL_RSC     "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/rsc/*")
file(GLOB _XAIEFAL_TOP     "${CMAKE_CURRENT_SOURCE_DIR}/../fal/src/*.hpp")

install(FILES ${_XAIEFAL_COMMON}  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xaiefal/common)
install(FILES ${_XAIEFAL_PROFILE} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xaiefal/profile)
install(FILES ${_XAIEFAL_RSC}     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xaiefal/rsc)
install(FILES ${_XAIEFAL_TOP}     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  TARGETS
  ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

install(
  EXPORT ${PROJECT_NAME}-targets
  NAMESPACE ${PROJECT_NAME}::
  COMPONENT runtime
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

# Support Find Package
include(CMakePackageConfigHelpers)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake @ONLY
  )

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion
  )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        COMPONENT runtime DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

# CodeQL static analysis
set(CODEQL_DB_DIR "${CMAKE_CURRENT_BINARY_DIR}/codeql-db" CACHE PATH
  "Directory where the CodeQL database is created")
set(CODEQL_RESULTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/codeql-results" CACHE PATH
  "Directory where CodeQL SARIF results are written")
set(CODEQL_QUERY_SUITE "codeql/cpp-queries:codeql-suites/cpp-security-extended.qls" CACHE STRING
  "CodeQL query suite to run")

find_program(CODEQL_EXECUTABLE_RAW codeql)
if(CODEQL_EXECUTABLE_RAW)
  get_filename_component(CODEQL_EXECUTABLE "${CODEQL_EXECUTABLE_RAW}" REALPATH)
else()
  set(CODEQL_EXECUTABLE CODEQL_EXECUTABLE_RAW-NOTFOUND)
endif()

if(CODEQL_EXECUTABLE)
  add_custom_target(codeql-db
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CODEQL_DB_DIR}"
    COMMAND ${CODEQL_EXECUTABLE} database create ${CODEQL_DB_DIR}
      --language=cpp
      --source-root=${CMAKE_CURRENT_SOURCE_DIR}
      --overwrite
      "--command=${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR} --clean-first"
    COMMENT "Creating CodeQL database at ${CODEQL_DB_DIR}"
    VERBATIM
  )

  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_custom_target(codeql
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CODEQL_RESULTS_DIR}"
    COMMAND ${CODEQL_EXECUTABLE} database analyze ${CODEQL_DB_DIR}
      --format=sarif-latest
      --output=${CODEQL_RESULTS_DIR}/codeql-results.sarif
      ${CODEQL_QUERY_SUITE}
    COMMAND ${Python3_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/../tools/sarif_to_text.py
      ${CODEQL_RESULTS_DIR}/codeql-results.sarif
      ${CODEQL_RESULTS_DIR}/codeql-results.txt
    COMMENT "Running CodeQL analysis — results at ${CODEQL_RESULTS_DIR}/"
    DEPENDS codeql-db
    VERBATIM
  )
else()
  add_custom_target(codeql
    COMMAND ${CMAKE_COMMAND} -E echo "ERROR: codeql not found in PATH. Install CodeQL CLI first."
    COMMAND ${CMAKE_COMMAND} -E false
  )
endif()
