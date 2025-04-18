#include <print>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <glm/glm.hpp>

#include "loader.hpp"

namespace mr {
inline namespace importer {
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
        Mesh mesh;

        // Process POSITION attribute
        auto posAttr = primitive.findAttribute("POSITION");
        if (posAttr == primitive.attributes.cend()) {
          std::println("primitive didn't contain positions");
          continue;
        }
        size_t posAccessorIndex = posAttr->accessorIndex;
        if (posAccessorIndex >= asset.accessors.size()) {
          std::println("primitive didn't contain position accessor");
          continue;
        }
        const Accessor& posAccessor = asset.accessors[posAccessorIndex];
        if (posAccessor.type != AccessorType::Vec3 || posAccessor.componentType != ComponentType::Float) {
          std::println("primitive's positions were in wrong format (not Vec3f)");
          continue;
        }
        if (!posAccessor.bufferViewIndex.has_value()) {
          std::println("primitive didn't contain buffer view");
          continue;
        }
        fastgltf::iterateAccessor<glm::vec3>(asset, posAccessor, [&](glm::vec3 v) {
          mesh.positions.push_back(v);
        });

        // Process indices
        assert(primitive.indicesAccessor.has_value());
        auto& idxAccessor = asset.accessors[primitive.indicesAccessor.value()];
        mesh.indices.resize(idxAccessor.count);
        fastgltf::copyFromAccessor<std::uint32_t>(asset, idxAccessor, mesh.indices.data());
      }
    }

    return result;
  }

  Asset import(std::filesystem::path path) {
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

    Asset asset;

    asset.meshes.emplace_back(
      mr::Mesh{
        .positions = {
          glm::vec3( .5f, .5f, .5f),  glm::vec3(-.5f, .5f, .5f),  glm::vec3(-.5f,-.5f, .5f), glm::vec3( .5f,-.5f, .5f), // v0,v1,v2,v3 (front)
          glm::vec3( .5f, .5f, .5f),  glm::vec3( .5f,-.5f, .5f),  glm::vec3( .5f,-.5f,-.5f), glm::vec3( .5f, .5f,-.5f), // v0,v3,v4,v5 (right)
          glm::vec3( .5f, .5f, .5f),  glm::vec3( .5f, .5f,-.5f),  glm::vec3(-.5f, .5f,-.5f), glm::vec3(-.5f, .5f, .5f), // v0,v5,v6,v1 (top)
          glm::vec3(-.5f, .5f, .5f),  glm::vec3(-.5f, .5f,-.5f),  glm::vec3(-.5f,-.5f,-.5f), glm::vec3(-.5f,-.5f, .5f), // v1,v6,v7,v2 (left)
          glm::vec3(-.5f,-.5f,-.5f),  glm::vec3( .5f,-.5f,-.5f),  glm::vec3( .5f,-.5f, .5f), glm::vec3(-.5f,-.5f, .5f), // v7,v4,v3,v2 (bottom)
          glm::vec3( .5f,-.5f,-.5f),  glm::vec3(-.5f,-.5f,-.5f),  glm::vec3(-.5f, .5f,-.5f), glm::vec3( .5f, .5f,-.5f)  // v4,v7,v6,v5 (back)
        },
        .indices = {
          0,  1,  2,   2,  3,  0,
          4,  5,  6,   6,  7,  4,
          8,  9, 10,  10, 11,  8,
          12, 13, 14,  14, 15, 12,
          16, 17, 18,  18, 19, 16,
          20, 21, 22,  22, 23, 20
        }
      }
    );

    return asset;
  }
}
}
