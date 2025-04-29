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
  // Set camera to FPS-like
  polyscope::view::setNavigateStyle(polyscope::NavigateStyle::FirstPerson);

  float xoffset = 0;
  for (int i = 0; i < meshes.size(); i++) {
	  auto& mesh = meshes[i];

	  int lodnumber = 0;
	  auto& lod = mesh.lods.size() - 1 < lodnumber ? mesh.lods.back() : mesh.lods[lodnumber];
	  auto& pos = mesh.positions;
	  auto ind = convertToArrayOfTriples(lod.indices);

	  for (int k = 0; k < mesh.transforms.size(); k++) {
		  auto* meshptr = polyscope::registerSurfaceMesh(std::format("Mesh {}{}; Instance {}", mesh.name, i, k), pos, ind);
		  meshptr->setTransform(mesh.transforms[k]);
		  meshptr->setEdgeWidth(1.0);  // Enable edge rendering by default
	  }
  }

  polyscope::show();
}
