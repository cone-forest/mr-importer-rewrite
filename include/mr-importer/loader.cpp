#include <print>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <glm/glm.hpp>

#include "loader.hpp"

namespace mr {
inline namespace importer {
  static std::optional<Mesh> getMeshFromPrimitive(const fastgltf::Asset &asset, const fastgltf::Primitive &primitive) {
    using namespace fastgltf;

    Mesh mesh;

    // Process POSITION attribute
    auto posAttr = primitive.findAttribute("POSITION");
    if (posAttr == primitive.attributes.cend()) {
      std::println("primitive didn't contain positions");
      return std::nullopt;
    }
    size_t posAccessorIndex = posAttr->accessorIndex;
    if (posAccessorIndex >= asset.accessors.size()) {
      std::println("primitive didn't contain position accessor");
      return std::nullopt;
    }
    const Accessor& posAccessor = asset.accessors[posAccessorIndex];
    if (posAccessor.type != AccessorType::Vec3 || posAccessor.componentType != ComponentType::Float) {
      std::println("primitive's positions were in wrong format (not Vec3f)");
      return std::nullopt;
    }
    if (!posAccessor.bufferViewIndex.has_value()) {
      std::println("primitive didn't contain buffer view");
      return std::nullopt;
    }
    fastgltf::iterateAccessor<glm::vec3>(asset, posAccessor, [&](glm::vec3 v) {
      mesh.positions.push_back(v);
    });

    // Process indices
    assert(primitive.indicesAccessor.has_value());
    auto& idxAccessor = asset.accessors[primitive.indicesAccessor.value()];
    mesh.lods[0].indices.resize(idxAccessor.count);
    fastgltf::copyFromAccessor<std::uint32_t>(asset, idxAccessor, mesh.lods[0].indices.data());

    return mesh;
  }

  static std::vector<Mesh> getMeshesFromGLTF(std::filesystem::path path) {
    using namespace fastgltf;

    std::vector<Mesh> result;

    auto [err, data] = GltfDataBuffer::FromPath(path);
    if (err != Error::None) {
      return {}; // Failed to load GLTF data
    }

    Parser parser;
    auto options = Options::LoadExternalBuffers | Options::LoadExternalImages;
    auto [error, asset] = parser.loadGltf(data, path.parent_path(), options);
    if (error != Error::None) {
      std::println("Failed to parse GLTF file");
      return {};
    }

    for (const auto& gltfMesh : asset.meshes) {
      for (const auto& primitive : gltfMesh.primitives) {
        auto mesh_opt = getMeshFromPrimitive(asset, primitive);
        if (mesh_opt.has_value()) {
	      result.emplace_back(std::move(mesh_opt.value()));
        }
      }
    }

    return result;
  }

  // Sequence:
  //   - parse gltf using fastgltf
  //   - Parallel:
  //     - mesh processing (DynamicParallel):
  //       - Sequence:
  //         - extract from gltf.bufferview's into PositionArray, IndexArray, VertexAttributesArray
  //         - optimize mesh data using meshoptimizer
  //     - material processing (DynamicParallel)
  //       - Sequence:
  //         - Parallel:
  //           - extract sampler data into SamplerData
  //           - extract from texture URI into ImageData using stb
  //         - compose into TextureData
  Asset import(std::filesystem::path path) {
    Asset asset;

    asset.meshes = getMeshesFromGLTF(path);

    return asset;
  }
}
}
