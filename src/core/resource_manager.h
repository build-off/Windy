#pragma once

#include "base.h"
#include "resource.h"
#include "resource_handle.h"
#include <string>
#include <typeindex>
#include <unordered_map>

namespace Windy::Core {

class ResourceManager {
public:
  template <typename T>
  ResourceHandle<T> Load(const std::string& resource_id) {}

  template <typename T>
  T *get_resource(std::string resource_id) {};

  template <typename T>
  bool has_resource(std::string resource_id) {};

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

} // namespace Windy::Core
