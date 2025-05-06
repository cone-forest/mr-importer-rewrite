#pragma once

#include "def.hpp"

namespace mr {
inline namespace importer {
template <typename T>
class Manager {
private:
  struct Entry {
    std::shared_ptr<std::atomic<std::shared_ptr<T>>> value =
    std::make_shared_for_overwrite<std::atomic<std::shared_ptr<T>>>();
    mr::Task<bool> task;
  };

  struct Handle {
    Manager &mgr;
    std::string key {};

    T& operator*() {
      auto &entry = mgr.table[key];
      if (!entry.value->load()) {
        entry.task->wait();
      }
      return *entry.value->load();
    }
  };

  std::unordered_map<std::string, Entry> table;

  Manager() {
    table.reserve(47);
  }

public:
  inline static Manager & get() {
    static Manager mgr;
    return mgr;
  }

  template <typename ...Args>
  Handle create(std::string name, Args ...args) {
    static auto prototype = mr::Sequence {
      mr::get_task_prototype<T, Args...>(),
      [name, this](T result) {
        auto resptr = std::make_shared<T>(std::move(result));
        resptr = table[name].value->exchange(std::move(resptr));
        return true;
      }
    };

    if constexpr (sizeof...(Args) > 1) {
      table[name] = {
        .task = mr::apply(prototype, std::forward_as_tuple(args...))
      };
    }
    else {
      table[name] = {
        .task = mr::apply(prototype, args...)
      };
    }

    table[name].task->schedule();
    return {*this, name};
  }
};
}
}
