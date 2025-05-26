#include <gtest/gtest.h>
#include <mr-importer/importer.hpp>

#define MR_DECLARE_IMPORT_PIPELINE_BEGIN(Type, ...) \
namespace mr { \
  template <> \
    struct TaskPrototypeBuilder<Type, __VA_ARGS__> { \
      inline static auto& create()

#define MR_DECLARE_IMPORT_PIPELINE_END \
  }; \
}

inline static auto f1 = [] (int x) -> int { return x+1; };
inline static auto f2 = [] (int x) -> int { return x+1; };
inline static auto f3 = [] (int x) -> int { return x+1; };

MR_DECLARE_IMPORT_PIPELINE_BEGIN(int, int)
{
  static auto prototype = mr::Sequence {
    f1, f2, f3
  };
  return prototype;
}
MR_DECLARE_IMPORT_PIPELINE_END

MR_DECLARE_IMPORT_PIPELINE_BEGIN(float, int)
{
  static auto prototype = mr::Sequence {
    f1, f2, f3, [](int x) -> float { return x; }
  };
  return prototype;
}
MR_DECLARE_IMPORT_PIPELINE_END

#if 0
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
#endif

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
