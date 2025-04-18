#include <gtest/gtest.h>
#include <mr-importer/importer.hpp>

namespace mr {
  template <>
    struct TaskPrototypeBuilder<int, int> {
      static ApplicableT auto & create() {
        static auto prototype = mr::Sequence {
          [] (int x) -> int { return x+1; },
          [] (int x) -> int { return x+1; },
          [] (int x) -> int { return x+1; },
        };
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
