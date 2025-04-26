#include <mr-importer/importer.hpp>
#include "render_polyscope.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: mr-importer-example <filename>");
    exit(47);
  }

  auto asset = mr::import(argv[1]);
  for (auto& mesh : asset.meshes) {
    mesh = mr::optimize(std::move(mesh));
  }

  render(asset.meshes);
}
