#include "mr-importer/assets.hpp"
#include <polyscope/render/materials.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

#include <glm/glm.hpp>
#include <mr-math/vec.hpp>

#include <cstdint>
#include <vector>
#include <array>

using mr::Vec3f;

inline std::vector<std::array<uint32_t, 3>> convertToArrayOfTriples(const std::vector<uint32_t>& input) {
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

inline void render(std::vector<mr::Mesh> meshes) {
  polyscope::init();

  // Disable ground
  polyscope::options::groundPlaneMode = polyscope::GroundPlaneMode::None;

  float xoffset = 0;
  for (int i = 0; i < meshes.size(); i++) {
	  auto& mesh = meshes[i];

	  // Compute min and max for this mesh
	  glm::vec3 min(std::numeric_limits<float>::max());
	  glm::vec3 max(std::numeric_limits<float>::lowest());
	  for (const auto& pos : mesh.positions) {
		  min.x = std::min(min.x, pos.x);
		  min.z = std::min(min.z, pos.z);
		  max.x = std::max(max.x, pos.x);
		  max.z = std::max(max.z, pos.z);
	  }
	  glm::vec3 delta = max - min;

	  for (int j = 0; j < mesh.lods.size(); j++) {
		  auto& lod = mesh.lods[j];
		  auto *meshptr = polyscope::registerSurfaceMesh(
        std::format("Mesh {}; LOD {}", i, j),
        mesh.positions,
        convertToArrayOfTriples(lod.indices));
		  meshptr->setPosition(glm::vec3(xoffset + i * std::abs(delta.x), 0, j * std::abs(delta.z)));
		  meshptr->setEdgeWidth(1.0);  // Enable edge rendering by default
	  }
	  xoffset += std::abs(delta.x);
  }

  polyscope::show();
}
