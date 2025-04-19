file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage("gh:SpartanJ/efsw#master")
CPMAddPackage("gh:spnda/fastgltf#main")
CPMAddPackage("gh:zeux/meshoptimizer#master")
CPMAddPackage("gh:4j-company/mr-contractor#master")
CPMAddPackage("gh:4j-company/mr-math#master")
CPMAddPackage("gh:raysan5/raylib#master")
CPMFindPackage(
  NAME glm
  GITHUB_REPOSITORY icaven/glm
  GIT_TAG master
  OPTIONS
    "GLM_BUILD_LIBRARY ON"
    "GLM_ENABLE_CXX_20 ON"
)

if (NOT TARGET libstb-image)
  # download a single file from stb
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/stb-src/stb/stb_image.h
    EXPECTED_HASH SHA256=594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3
  )
  add_library(libstb-image INTERFACE "")
  target_include_directories(libstb-image INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/_deps/stb-src/)
endif()

set(MR_IMPORTER_DEPS fastgltf::fastgltf meshoptimizer libstb-image efsw mr-contractor-lib mr-math-lib glm::glm)
