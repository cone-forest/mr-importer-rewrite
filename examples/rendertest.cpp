#include <mr-importer/importer.hpp>
#include "render.hpp"

int main() {
  auto asset = mr::import("/home/michael/Development/Personal/mr-importer-rewrite/ABeautifulGame/ABeautifulGame.gltf");

  render(asset.meshes[0].positions, asset.meshes[0].indices);
}
