#pragma once

#include <functional>
#include <type_traits>

namespace mr {
  template <typename T>
  concept Hashable = requires(T a) {
    { std::hash<std::remove_cvref_t<T>>{}(a) } -> std::convertible_to<size_t>;
  };

  template <typename... Args>
  struct hash {
    size_t operator()(Args ...args) {
      size_t result = 0;
      (process_arg(result, std::forward<Args>(args)), ...);
      return result;
    }

  private:
    template <typename T> void process_arg(size_t& seed, T&& arg) {}

    template <Hashable T>
      void process_arg(size_t& seed, T&& arg) {
        size_t hash = std::hash<std::remove_cvref_t<T>>{}(std::forward<T>(arg));
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
  };
}
