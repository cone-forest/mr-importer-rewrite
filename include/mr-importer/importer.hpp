#pragma once

#include "def.hpp"
#include "assets.hpp"
#include "atomic_unique_ptr.hpp"
#include "manager.hpp"
#include "loader.hpp"
#include "optimizer.hpp"
#include "options.hpp"

namespace mr {
inline namespace importer {
  inline Asset import(std::filesystem::path path, uint32_t options = Options::All) {
    auto asset = load(path);

    if (options & Options::OptimizeMeshes) {
      for (auto& mesh : asset.meshes) {
        mesh = mr::optimize(std::move(mesh));
      }
    }

    return asset;
  }
}
}
