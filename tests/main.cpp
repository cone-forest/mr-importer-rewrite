#include <gtest/gtest.h>
#include <mr-importer/importer.hpp>

inline static auto f1 = [] (int x) -> int { return x+1; };
inline static auto f2 = [] (int x) -> int { return x+1; };
inline static auto f3 = [] (int x) -> int { return x+1; };

namespace mr {
  template <>
    struct TaskPrototypeBuilder<int, int> {
      inline static auto & create() {
        static auto prototype = mr::Sequence {
          f1, f2, f3
        };
        return prototype;
      }
    };
  template <>
    struct TaskPrototypeBuilder<float, int> {
      inline static auto & create() {
        static auto prototype = mr::Sequence {
          f1, f2, f3, [](int x) -> float { return x; }
        };
        return prototype;
      }
    };
}

TEST(ManagerTestInt, SingleIntResource) {
  mr::Manager<int> &mgr = mr::Manager<int>::get();
  EXPECT_EQ(3, *mgr.create("first",  0));
}

TEST(ManagerTestFloat, FirstFloatResource) {
  mr::Manager<float> &mgr = mr::Manager<float>::get();
  EXPECT_EQ(3, (int)*mgr.create("first",  0));
}

TEST(ManagerTestFloat, SecondFloatResource) {
  mr::Manager<float> &mgr = mr::Manager<float>::get();
  EXPECT_EQ(4, (int)*mgr.create("second", 1));
}

TEST(ManagerTestFloat, ThirdFloatResource) {
  mr::Manager<float> &mgr = mr::Manager<float>::get();
  EXPECT_EQ(5, (int)*mgr.create("third",  2));
}
