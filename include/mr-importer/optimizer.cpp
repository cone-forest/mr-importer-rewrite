#include "def.hpp"
#include "mr-importer/assets.hpp"
#include <meshoptimizer.h>
#include "optimizer.hpp"

namespace mr {
inline namespace importer {
  std::pair<IndexArray, IndexArray> generate_lod(
    const PositionArray &positions,
    const IndexArray &original_indices,
    const std::span<meshopt_Stream> &streams,
    int lod_index)
  {
    static constexpr float target_error = 0.05f;
    static constexpr auto k = [](size_t lod_level) {
      constexpr std::array ratios = { 1.0f, 0.3f, 0.1f, 0.05f, 0.01f, 0.005f };
      int idx = std::round((float)lod_level / (Mesh::lodcount-1) * (ratios.size() - 1));
      return ratios[idx];
    };

    IndexArray result_indices;
    IndexArray result_shadow_indices;

    size_t original_index_count = original_indices.size();

    result_indices.resize(original_index_count);
    float lod_error = 0.f;
    result_indices.resize(
      meshopt_simplify(
        result_indices.data(),
        original_indices.data(), original_indices.size(),
        (float*)positions.data(), positions.size(), sizeof(Position),
        original_index_count * k(lod_index), target_error,
        /* Don't simplify "free" edges */ meshopt_SimplifyLockBorder |
        /* Treat last lod as sparse    */ (lod_index == Mesh::lodcount - 1 ? meshopt_SimplifySparse : 0),
        &lod_error)
    );

    meshopt_optimizeVertexCache(result_indices.data(), result_indices.data(), result_indices.size(), positions.size());

    result_shadow_indices.resize(result_indices.size());
    meshopt_generateShadowIndexBufferMulti(result_shadow_indices.data(),
                                           result_indices.data(), result_indices.size(),
                                           positions.size(),
                                           streams.data(), streams.size());
    meshopt_optimizeVertexCache(result_shadow_indices.data(), result_shadow_indices.data(), result_shadow_indices.size(), positions.size());

    return {result_indices, result_shadow_indices};
  }

  Mesh optimize(Mesh mesh) {
    if (mesh.attributes.empty()) {
      mesh.attributes.resize(mesh.positions.size());
    }

    Mesh result;
    std::array streams = {
      meshopt_Stream{mesh.positions.data(), sizeof(Position), sizeof(Position)},
      meshopt_Stream{mesh.attributes.data(), sizeof(VertexAttributes), sizeof(VertexAttributes)},
    };

    // improve vertex locality
    meshopt_optimizeVertexCache(mesh.lods[0].indices.data(), mesh.lods[0].indices.data(), mesh.lods[0].indices.size(), mesh.positions.size());

    // optimize overdraw
    meshopt_optimizeOverdraw(mesh.lods[0].indices.data(), mesh.lods[0].indices.data(), mesh.lods[0].indices.size(),
                             (float*)mesh.positions.data(), mesh.positions.size(), sizeof(Position), 1.05f);

    IndexArray remap;
    remap.resize(mesh.lods[0].indices.size());
    size_t vertex_count = meshopt_generateVertexRemapMulti(
      remap.data(),
      mesh.lods[0].indices.data(),
      mesh.lods[0].indices.size(),
      mesh.positions.size(),
      streams.data(),
      streams.size());

    result.lods[0].indices.resize(mesh.lods[0].indices.size());
    result.positions.resize(vertex_count);
    result.attributes.resize(vertex_count);

    meshopt_remapIndexBuffer(result.lods[0].indices.data(), mesh.lods[0].indices.data(), mesh.lods[0].indices.size(), remap.data());
    meshopt_remapVertexBuffer(result.positions.data(), mesh.positions.data(), mesh.positions.size(), sizeof(Position), remap.data());
    meshopt_remapVertexBuffer(result.attributes.data(), mesh.attributes.data(), mesh.positions.size(), sizeof(VertexAttributes), remap.data());

    meshopt_optimizeVertexFetchRemap(
      remap.data(),
      result.lods[0].indices.data(), result.lods[0].indices.size(),
      result.positions.size());

    // NOTE: we run remap functions the second time as recommended by docs
    meshopt_remapIndexBuffer(result.lods[0].indices.data(), result.lods[0].indices.data(), result.lods[0].indices.size(), remap.data());
    meshopt_remapVertexBuffer(result.positions.data(), result.positions.data(), result.positions.size(), sizeof(Position), remap.data());
    meshopt_remapVertexBuffer(result.attributes.data(), result.attributes.data(), result.attributes.size(), sizeof(VertexAttributes), remap.data());

    result.lods[0].shadow_indices.resize(result.lods[0].indices.size());
    meshopt_generateShadowIndexBufferMulti(result.lods[0].shadow_indices.data(),
                                           result.lods[0].indices.data(), result.lods[0].indices.size(),
                                           result.positions.size(),
                                           streams.data(), streams.size());

    float lodScale = meshopt_simplifyScale((float*)result.positions.data(), result.positions.size(), sizeof(Position));

    // LOD generation
    for (int i = 1; i < Mesh::lodcount; i++) {
      std::tie(result.lods[i].indices, result.lods[i].shadow_indices) = generate_lod(result.positions, result.lods[0].indices, streams, i);
    }

    return result;
  }
}
}
