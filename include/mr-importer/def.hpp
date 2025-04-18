#pragma once

#include <stb/stb_image.h>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <meshoptimizer.h>

#include <mr-contractor/contractor.hpp>
#include <mr-math/math.hpp>

namespace mr {
  template <typename Ret, typename... Args> struct TaskPrototypeBuilder;

  template <typename Ret, typename... Args>
  ApplicableT auto & get_task_prototype() {
    return TaskPrototypeBuilder<Ret, Args...>::create();
  }

  template <typename ResultT, typename ...Args>
  auto make_task(Args ...args) {
    if constexpr (sizeof...(args) > 1) {
      return mr::apply(
        get_task_prototype<ResultT, Args...>(),
        std::forward_as_tuple<Args...>(args...)
      );
    }
    else {
      return mr::apply(
        get_task_prototype<ResultT, Args...>(),
        args...
      );
    }
  }

inline namespace importer {
}
}
