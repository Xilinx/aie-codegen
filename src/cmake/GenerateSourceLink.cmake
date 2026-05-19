# Writes aie_codegen.sourcelink.json for /SOURCELINK (invoked from AieCodegenSourceLink.cmake).
#
# SPDX-License-Identifier: MIT

if(NOT DEFINED OUT_FILE OR NOT DEFINED GIT_EXECUTABLE OR NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "GenerateSourceLink.cmake: missing OUT_FILE, GIT_EXECUTABLE, or SOURCE_DIR")
endif()

execute_process(
  COMMAND "${GIT_EXECUTABLE}" rev-parse --show-toplevel
  WORKING_DIRECTORY "${SOURCE_DIR}"
  OUTPUT_VARIABLE _git_root
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_VARIABLE _git_root_err
  RESULT_VARIABLE _git_root_rc
)
if(_git_root_rc OR NOT _git_root)
  message(FATAL_ERROR "GenerateSourceLink.cmake: could not determine git root (${_git_root_err})")
endif()

if(DEFINED GIT_SHA_OVERRIDE AND NOT "${GIT_SHA_OVERRIDE}" STREQUAL "")
  set(_git_sha "${GIT_SHA_OVERRIDE}")
else()
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${_git_root}"
    OUTPUT_VARIABLE _git_sha
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE _git_sha_err
    RESULT_VARIABLE _git_sha_rc
  )
  if(_git_sha_rc OR NOT _git_sha)
    message(FATAL_ERROR "GenerateSourceLink.cmake: could not read HEAD (${_git_sha_err})")
  endif()
endif()

execute_process(
  COMMAND "${GIT_EXECUTABLE}" config --get remote.origin.url
  WORKING_DIRECTORY "${_git_root}"
  OUTPUT_VARIABLE _git_remote
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

string(REPLACE "\\" "/" _local_root "${_git_root}")
if(NOT _local_root MATCHES "/$")
  set(_local_root "${_local_root}/*")
else()
  set(_local_root "${_local_root}*")
endif()

set(_repo_url "")
if(_git_remote MATCHES "^git@([^:]+):(.+)$")
  set(_host "${CMAKE_MATCH_1}")
  set(_path "${CMAKE_MATCH_2}")
  string(REGEX REPLACE "\\.git$" "" _path "${_path}")
  set(_repo_url "https://${_host}/${_path}")
elseif(_git_remote MATCHES "^https?://(.+)$")
  set(_repo_url "${_git_remote}")
  string(REGEX REPLACE "\\.git$" "" _repo_url "${_repo_url}")
endif()

if(NOT _repo_url)
  message(FATAL_ERROR "GenerateSourceLink.cmake: unsupported or missing origin URL '${_git_remote}'")
endif()

set(_remote_prefix "${_repo_url}/raw/${_git_sha}/*")

file(WRITE "${OUT_FILE}"
  "{\n  \"documents\": {\n    \"${_local_root}\": \"${_remote_prefix}\"\n  }\n}\n")
