#pragma once

#include "base.h"
#include "resource.h"
#include "resource_handle.h"
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace Windy::Core {

class ResourceManager {
public:
  template <typename T>
  ResourceHandle<T> Load(const std::string& resource_id) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto& type_resources = resources[std::type_index(typeid(T))];
    auto  it             = type_resources.find(resource_id);
    if (it != type_resources.end()) {
      // FIX: increment the reference count of the resource
      // ref_counts[resource_id]++;
      return ResourceHandle<T>(resource_id, this);
    }

    auto resource = CreateRef<T>(resource_id);
    if (!resource->load()) {
      return ResourceHandle<T>();
    }

    // loaded and not in cache, add it and return the handle
    type_resources[resource_id] = resource;
    return ResourceHandle<T>(resource_id, this);
  };

  template <typename T>
  T *get_resource(const std::string& resource_id) {
    auto& type_resources = resources[std::type_index(typeid(T))];
    auto  it             = type_resources.find(resource_id);
    if (it != type_resources.end()) {
      return static_cast<T *>(it->second.get());
    }
    return nullptr;
  };

  template <typename T>
  bool has_resource(std::string resource_id) {
    auto resource_it = resources.find(std::type_index(typeid(T)));
    return resource_it != resources.end();
  };

private:
  struct ResourceData {
    Ref<Resource> resource;
    int           ref_count;
  };
  std::unordered_map<std::type_index,
                     std::unordered_map<std::string, Ref<Resource>>>
      resources;
  std::unordered_map<std::type_index,
                     std::unordered_map<std::string, ResourceData>>
      ref_counts;
};

template <typename T>
T *ResourceHandle<T>::get() const {
  if (!resource_manager) return nullptr;
  return resource_manager->get_resource<T>(resource_id);
}

template <typename T>
bool ResourceHandle<T>::is_valid() const {
  return resource_manager && resource_manager->has_resource<T>(resource_id);
}

} // namespace Windy::Core
