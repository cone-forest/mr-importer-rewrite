#define GLM_ENABLE_EXPERIMENTAL
#include <polyscope/polyscope.h>
#include <polyscope/surface_mesh.h>

#include <glm/glm.hpp>
#include <mr-math/vec.hpp>

#include <cstdint>
#include <vector>
#include <array>

using mr::Vec3f;

std::vector<std::array<uint32_t, 3>> convertToArrayOfTriples(const std::vector<uint32_t>& input) {
  const uint32_t newSize = input.size() / 3;
  std::vector<std::array<uint32_t, 3>> result;
  result.reserve(newSize);

  for (uint32_t i = 0; i < input.size(); i += 3) {
    result.push_back({input[i], input[i+1], input[i+2]});
  }

  return result;
}

inline void render(std::vector<glm::vec3> positions, std::vector<uint32_t> indices) {
  polyscope::init();
  auto mesh = polyscope::registerSurfaceMesh("my mesh", positions, convertToArrayOfTriples(indices));
  polyscope::show();
}
