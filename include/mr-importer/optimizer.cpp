#include "def.hpp"
#include "optimizer.hpp"

namespace mr {
inline namespace importer {
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
	meshopt_generateShadowIndexBufferMulti(result.lods[0].shadow_indices.data(), result.lods[0].indices.data(), result.lods[0].indices.size(), result.positions.size(), streams.data(), streams.size());

    // LOD generation

    // so that last lod is a sparse subset of the original mesh
    constexpr auto k = [](size_t lod_level) {
      std::array ratios = { 1.0f, 0.6f, 0.4f, 0.25f, 0.1f, 0.02f };
      return ratios[std::round((float)lod_level / (Mesh::lodcount-1) * (ratios.size() - 1))];
    };
	size_t original_index_count = result.lods[0].indices.size();
	float target_error = 0.05f;
    for (size_t i = 1; i < Mesh::lodcount; i++) {
		result.lods[i].indices.resize(original_index_count);
		float lod_error = 0.f;
		result.lods[i].indices.resize(
			meshopt_simplify(
				result.lods[i].indices.data(),
				result.lods[i - 1].indices.data(), result.lods[i - 1].indices.size(),
				(float*)result.positions.data(), result.positions.size(), sizeof(Position),
				original_index_count * k(i), target_error,
                /* Don't simplify "free" edges */ meshopt_SimplifyLockBorder |
                /* Treat last lod as sparse */ (i != Mesh::lodcount - 1 ? 0 : meshopt_SimplifySparse),
                &lod_error)
		);

      result.lods[i].shadow_indices.resize(result.lods[i].indices.size());
      meshopt_generateShadowIndexBufferMulti(result.lods[i].shadow_indices.data(), result.lods[i].indices.data(), result.lods[i].indices.size(), result.positions.size(), streams.data(), streams.size());
    }

    return result;
  }
}
}
