#include <gtest/gtest.h>
#include <mr-importer/importer.hpp>

inline static auto f1 = [] (int x) -> int { return x+1; };
inline static auto f2 = [] (int x) -> int { return x+1; };
inline static auto f3 = [] (int x) -> int { return x+1; };

inline static auto prototype = mr::Sequence {
  f1, f2, f3
};

namespace mr {
  template <>
    struct TaskPrototypeBuilder<int, int> {
      inline static auto & create() {
        return prototype;
      }
    };
}

TEST(ImporterTest, ABeautifulGame) {
  mr::Manager<int> mgr;
  auto handle = mgr.create("first", 0);
  EXPECT_EQ(*handle, 3);
  EXPECT_TRUE(true);
}
