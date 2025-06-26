#pragma once

#include <atomic>
#include <memory_resource>
#include <functional>
#include <stdexcept>

#include <folly/synchronization/Rcu.h>
#include <folly/concurrency/ConcurrentHashMap.h>

#include <efsw/efsw.hpp>

#include <mr-contractor/contractor.hpp>

#include "hash.hpp"

namespace mr {
using AssetId = std::uint64_t;

template<typename T, typename Id> T import(const Id& id);

template <typename T> struct Manager;

template <typename T>
struct Handle {
  const Manager<T> &manager;
  AssetId id;

  Handle(const Manager<T>& mgr, AssetId asset_id)
  : manager(mgr), id(asset_id) {}

  template <typename F>
  bool with(F&& func) {
    const auto &entry = manager.find(id);
    if (T* ptr = entry.value.load(std::memory_order_acquire); ptr != nullptr) {
      std::scoped_lock<folly::rcu_domain> guard {folly::rcu_default_domain()};
      std::invoke(func, *ptr);
      return true;
    }

    return false;
  }
};

template <typename T>
struct Manager {
  struct Entry {
    std::atomic<T*> value = nullptr;

    Entry() = default;
    Entry(Entry &&other) {
      value.store(other.value.load(std::memory_order_acquire));
    }
    Entry & operator=(Entry &&other) {
      value.store(other.value.load(std::memory_order_acquire));
      return *this;
    }
  };

  static inline constexpr size_t _max_elements = 1024;
  static inline constexpr size_t _buffer_size = sizeof(T) * _max_elements;

  static inline std::array<std::byte, _buffer_size> _memory_buffer {};
  static inline std::pmr::monotonic_buffer_resource _memory_resource {_memory_buffer.data(), _buffer_size};
  static inline std::pmr::synchronized_pool_resource _memory_pool_resource {&_memory_resource};
  static inline std::pmr::polymorphic_allocator<T> _allocator {&_memory_pool_resource};

  static Manager& get() {
    static Manager instance;
    return instance;
  }

  folly::ConcurrentHashMap<AssetId, Entry> _table {_max_elements};

  template<typename ...Args>
  Handle<T> create(Args&& ...args) {
    AssetId asset_id = mr::hash<Args...>{}(args...);

    auto it = _table.find(asset_id);
    if (it != _table.end()) {
      return Handle<T>{*this, asset_id};
    }

    _table.insert(asset_id, init_entry(std::forward<Args>(args)...));

    return Handle<T>{*this, asset_id};
  }

  const Entry &find(AssetId id) const {
    auto it = _table.find(id);
    if (it == _table.end()) {
      throw std::runtime_error("Asset not found");
    }
    return it->second;
  }

private:
  Manager() noexcept = default;

  ~Manager() {
    for (auto& [id, entry] : _table) {
      if (T* ptr = entry.value.load(std::memory_order_acquire)) {
        ptr->~T();
        _allocator.deallocate(ptr, 1);
      }
    }
  }

  template <typename ...Args>
  static Entry init_entry(Args && ...args) {
    Entry entry {};
    T* ptr = _allocator.allocate(1);
    assert(ptr != nullptr);

    new (ptr) T(std::forward<Args>(args)...);

    entry.value.store(ptr, std::memory_order_release);
    return entry;
  }
};

} // namespace mr
