#include "importer.hpp"

namespace mr {
  inline namespace importer {
    Asset::Asset(const std::filesystem::path &path) {
      *this = import(path);
    }
  }
}
