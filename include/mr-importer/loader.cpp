#include <print>

#include "mr-importer/def.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    mesh.positions.reserve(posAccessor.count);
    fastgltf::iterateAccessor<glm::vec3>(asset, posAccessor, [&](glm::vec3 v) {
      mesh.positions.push_back(v);
    });

    // Process indices
    assert(primitive.indicesAccessor.has_value());
    mesh.lods.resize(1);
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
      std::println("Error code: {}", (int)error);
      return {};
    }

    std::vector<std::vector<glm::mat4>> transforms;
    transforms.resize(asset.meshes.size());
    fastgltf::iterateSceneNodes(asset, 0, fastgltf::math::fmat4x4(),
      [&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix) {
        if (node.meshIndex.has_value()) {
          transforms[*node.meshIndex].push_back(glm::make_mat4(matrix.data()));
        }
      }
    );

    for (int i = 0; i < asset.meshes.size(); i++) {
      const fastgltf::Mesh& gltfMesh = asset.meshes[i];
	  for (int j = 0; j < gltfMesh.primitives.size(); j++) {
		  const auto& primitive = gltfMesh.primitives[j];
		  auto mesh_opt = getMeshFromPrimitive(asset, primitive);
		  if (mesh_opt.has_value()) {
			  mesh_opt->transforms = transforms[i];
			  mesh_opt->name = gltfMesh.name;
			  result.emplace_back(std::move(mesh_opt.value()));
		  }
      }
    }

    return result;
  }

  static std::optional<ImageData> loadImageFromURI(fastgltf::sources::URI& filePath) {
    assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
    assert(filePath.uri.isLocalPath()); // We're only capable of loading

    int width, height, nrChannels;

    Color* dataptr = (Color*)stbi_load(filePath.uri.c_str(), &width, &height, &nrChannels, 4);

    if (not dataptr) {
      return std::nullopt;
    }

    return ImageData{
      .pixels = std::unique_ptr<Color[]>(dataptr),
      .width = (uint32_t)width,
      .height = (uint32_t)height,
    };
  }

  static std::optional<ImageData> loadImageFromVector(fastgltf::sources::Vector& vector) {
    int width, height, nrChannels;

    Color* dataptr = (Color*)stbi_load_from_memory((unsigned char *)vector.bytes.data(), (int)vector.bytes.size(), &width, &height, &nrChannels, 4);

    if (not dataptr) {
      return std::nullopt;
    }

    return ImageData{
      .pixels = std::unique_ptr<Color[]>(dataptr),
      .width = (uint32_t)width,
      .height = (uint32_t)height,
    };
  }

  static std::optional<ImageData> getImagesFromGLTF(const fastgltf::Asset &asset, const fastgltf::Image &image) {
    ImageData newImage {};

    int width, height, nrChannels;

    std::visit(fastgltf::visitor {
      [](auto &arg){},
      [&](fastgltf::sources::URI& filePath) {
      },
      [&](fastgltf::sources::Vector& vector) {
      },
      [&](fastgltf::sources::BufferView& view) {
      }
    }, image.data);

    return newImage;

    /*
    std::visit(
      fastgltf::visitor {
        [](auto& arg) {},
        [&](fastgltf::sources::URI& filePath) {
          assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
          assert(filePath.uri.isLocalPath()); // We're only capable of loading
          // local files.

          const std::string path(filePath.uri.path().begin(),
                                 filePath.uri.path().end()); // Thanks C++.
          unsigned char* dataptr = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
          if (dataptr) {
            VkExtent3D imagesize;
            imagesize.width = width;
            imagesize.height = height;
            imagesize.depth = 1;
          }
        },
        [&](fastgltf::sources::Vector& vector) {
          unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                                                      &width, &height, &nrChannels, 4);
          if (data) {
            VkExtent3D imagesize;
            imagesize.width = width;
            imagesize.height = height;
            imagesize.depth = 1;
          }
        },
        [&](fastgltf::sources::BufferView& view) {
          auto& bufferView = asset.bufferViews[view.bufferViewIndex];
          auto& buffer = asset.buffers[bufferView.bufferIndex];

          std::visit(fastgltf::visitor { // We only care about VectorWithMime here, because we
            // specify LoadExternalBuffers, meaning all buffers
            // are already loaded into a vector.
            [](auto& arg) {},
            [&](fastgltf::sources::Vector& vector) {
              unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                          static_cast<int>(bufferView.byteLength),
                                                          &width, &height, &nrChannels, 4);
              if (data) {
                VkExtent3D imagesize;
                imagesize.width = width;
                imagesize.height = height;
                imagesize.depth = 1;
              }
            } },
                     buffer.data);
        },
      },
      image.data);
    */
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
  Asset load(std::filesystem::path path) {
    Asset asset;

    asset.meshes = getMeshesFromGLTF(path);

    return asset;
  }
}
}
