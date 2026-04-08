#pragma once

#include <memory>
#include <utility>

#define WD_STRINGIFY_MACRO(x) #x

namespace Windy {
// abstraction over shared and unique pointers
// to manage changes to implementation, in a single place
template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace Windy
