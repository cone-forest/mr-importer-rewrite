# Opinionated asset importer

## Features
- Geometry import and optimization
- Automatic LOD generation

## Build
```bash
git clone https://github.com/4j-company/mr-importer
cd mr-importer
cmake -S . -B build
cmake --build build
```

## Dependencies:
- Filewatcher: [efsw](https://github.com/SpartanJ/efsw)
- GLTF parser: [fastgltf](https://github.com/spnda/fastgltf)
- Mesh Processor: [meshoptimizer](https://github.com/zeux/meshoptimizer)
- Parallel task execution: [mr-contractor](https://github.com/4j-company/mr-contractor)
- Math library: [mr-math](https://github.com/4j-company/mr-math)

NOTE: automatically downloaded (if not found) via CMake script

## TODO
- Features:
    - Load textures (with samplers)
    - Load materials
    - Hot reloading (using efsw)
    - Asset manager (including dependency tracking for hot reloading)
    - Async loading (using mr-contractor)
    - Performance statistics (?)
- Performance:
    - Try to give out `std::span` instead of `std::vector` for positions, indices, etc.

