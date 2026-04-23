#pragma once

#include "resource_manager.h"

namespace Windy::Core {
class ResourceManager;
template <typename T>
class ResourceHandle {
public:
  ResourceHandle() : resource_manager{nullptr} {};
  ResourceHandle(const std::string& id, ResourceManager *manager)
      : resource_id{id}, resource_manager{manager} {}

  T *get() const {
    if (!resource_manager) return nullptr;
    return resource_manager->get_resource<T>(resource_id);
  };

  bool is_valid() const {
    return resource_manager && resource_manager->has_resource<T>(resource_id);
  };

  const std::string& get_id() { return resource_id; }
  T&                 operator->() const { return get(); }
  T&                 operator*() const { return *get(); };
                     operator bool() const { is_valid(); }

private:
  std::string      resource_id;
  ResourceManager *resource_manager;
};

} // namespace Windy::Core
