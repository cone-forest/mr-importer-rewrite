#pragma once

#include <atomic>

namespace mr {
inline namespace importer {

  template<typename T>
    class AtomicUniquePtr {
    private:
      std::atomic<T*> ptr;

    public:
      // Constructors
      explicit AtomicUniquePtr(T* p = nullptr) noexcept : ptr(p) {}

      // Destructor - Releases ownership of the managed object
      ~AtomicUniquePtr() {
        delete ptr.exchange(nullptr, std::memory_order_acq_rel);
      }

      // Disallow copying
      AtomicUniquePtr(const AtomicUniquePtr&) = delete;
      AtomicUniquePtr& operator=(const AtomicUniquePtr&) = delete;

      // Move operations could be implemented but are omitted for simplicity
      AtomicUniquePtr(AtomicUniquePtr&&) = delete;
      AtomicUniquePtr& operator=(AtomicUniquePtr&&) = delete;

      // Atomically swap the managed object and delete the previous one
      void swap(T* new_ptr) noexcept {
        delete ptr.exchange(new_ptr, std::memory_order_acq_rel);
      }

      // Get the current raw pointer (use with caution)
      T* get() const noexcept {
        return ptr.load(std::memory_order_acquire);
      }

      // Additional helper methods
      T* operator->() const noexcept { return get(); }
      T& operator*() const noexcept { return *get(); }
    };
}
}
