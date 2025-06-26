#include <filesystem>
#include <mr-importer/importer.hpp>
#include "render_polyscope.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: mr-importer-example <filename>");
    exit(47);
  }

  auto handle = mr::Manager<mr::Asset>::get().create(std::filesystem::path(argv[1]));
  handle.with(
    [&](const mr::Asset &asset) {
      render(asset.meshes);
    }
  );
}
