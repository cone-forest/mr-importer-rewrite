file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

find_package(folly REQUIRED)
find_package(efsw REQUIRED)
find_package(meshoptimizer REQUIRED)
find_package(fastgltf REQUIRED)
find_package(stb REQUIRED)

CPMAddPackage("gh:4j-company/mr-contractor#master")
CPMAddPackage("gh:nmwsharp/polyscope#master")

# CPMAddPackage("gh:SpartanJ/efsw#master")
# CPMAddPackage("gh:zeux/meshoptimizer#master")
# CPMAddPackage("gh:4j-company/fastgltf#main")
# if (NOT TARGET glm)
# cmake_policy(SET CMP0079 NEW)
# CPMFindPackage(
#   NAME glm
#   GITHUB_REPOSITORY icaven/glm
#   GIT_TAG master
#   OPTIONS
#     "GLM_BUILD_LIBRARY ON"
#     "GLM_ENABLE_CXX_20 ON"
# )
# endif()

set(MR_IMPORTER_DEPS
  meshoptimizer::meshoptimizer
  fastgltf::fastgltf
  efsw::efsw
  stb::stb
  glm::glm
  folly::folly
  mr-contractor-lib
)
