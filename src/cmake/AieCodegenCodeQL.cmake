###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

set(CODEQL_DB_DIR "${CMAKE_CURRENT_BINARY_DIR}/codeql-db" CACHE PATH
  "Directory where the CodeQL database is created")
set(CODEQL_RESULTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/codeql-results" CACHE PATH
  "Directory where CodeQL SARIF results are written")
set(CODEQL_QUERY_SUITE "codeql/cpp-queries:codeql-suites/cpp-security-extended.qls" CACHE STRING
  "CodeQL query suite to run")

find_program(CODEQL_EXECUTABLE codeql)

if(CODEQL_EXECUTABLE)
  add_custom_target(codeql-db
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CODEQL_DB_DIR}"
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
