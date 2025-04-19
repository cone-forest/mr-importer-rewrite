#include <mr-importer/importer.hpp>
#include "render_polyscope.hpp"

int main() {
  auto asset = mr::import("D:/Development/mr-importer-rewrite/ABeautifulGame/ABeautifulGame.gltf");
  for (auto& mesh : asset.meshes) {
      mesh = mr::optimize(std::move(mesh));
  }

  render(asset.meshes);
}
