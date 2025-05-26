#pragma once

#include <string>
#include <atomic>

#include "def.hpp"

namespace mr {
inline namespace importer {
template <typename T>
class Manager {
private:
  struct Entry {
    std::atomic<T*> value = nullptr;
    std::pair<T, T> buffers;
    mr::Task<bool> task;
  };

  struct Handle {
    Manager &mgr;
    std::string key {};

    T& operator*() {
      auto &entry = mgr.table[key];
      if (entry.value.load() == nullptr) {
        entry.task->wait();
      }
      return *entry.value;
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
        auto &entry = table[name];

        if (entry.value.load() == &entry.buffers.first) {
          entry.buffers.second = std::move(result);
          entry.value = &entry.buffers.second;
        }
        else {
          entry.buffers.first = std::move(result);
          entry.value = &entry.buffers.first;
        }

        return true;
      }
    };

    auto &entry = table[name];
    if constexpr (sizeof...(Args) > 1) {
      entry.task = mr::apply(prototype, std::forward_as_tuple(args...));
    }
    else {
      entry.task = mr::apply(prototype, args...);
    }
    entry.task->schedule();
    return {*this, name};
  }
};
}
}
