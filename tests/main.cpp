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

TEST(ImporterTest, SingleResource) {
  mr::Manager<int> mgr;
  EXPECT_EQ(3, *mgr.create("first",  0));
}

TEST(ImporterTest, ManyResources) {
  mr::Manager<int> mgr;
  EXPECT_EQ(3, *mgr.create("first",  0));
  EXPECT_EQ(4, *mgr.create("second", 1));
  EXPECT_EQ(5, *mgr.create("third",  2));
}
