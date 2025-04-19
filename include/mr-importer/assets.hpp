#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
#include <mr-math/vec.hpp>
#include <mr-math/color.hpp>

namespace mr {
inline namespace importer {
  using Position = glm::vec3;
  using Index = std::uint32_t;
  struct VertexAttributes {
    Color color;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
  };

  // mesh-related data
  struct PositionArray : std::vector<Position> {
    using std::vector<Position>::vector;
    using std::vector<Position>::operator=;
  };

  struct IndexArray : std::vector<Index> {
    using std::vector<Index>::vector;
    using std::vector<Index>::operator=;
  };

  struct VertexAttributesArray : std::vector<VertexAttributes> {
    using std::vector<VertexAttributes>::vector;
    using std::vector<VertexAttributes>::operator=;
  };

  struct Mesh {
	static inline constexpr size_t lodcount = 4;
    static_assert(lodcount >= 1, "lodcount should be at least 1 (lods[0] is not simplified)");

    struct LOD {
	  IndexArray indices;
	  IndexArray shadow_indices;
    };

    PositionArray positions;
    VertexAttributesArray attributes;
    std::array<LOD, lodcount> lods;
  };

  // material-related data
  struct ImageData {
    std::vector<Color> pixels;
    uint32_t width;
    uint32_t height;
  };
  struct SamplerData {
    std::vector<Vec2f> texcoords;
  };
  struct TextureData {
    ImageData image;
    SamplerData sampler;
  };
  struct MaterialData {
    std::vector<TextureData> textures;
  };

  // TODO: animation-related data

  struct Asset {
    std::vector<Mesh> meshes;
    std::vector<MaterialData> materials;
  };
}
}
