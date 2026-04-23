#pragma once

namespace Windy::Core {
class ResourceManager;
template <typename T>
class ResourceHandle {
public:
  ResourceHandle() : resource_manager{nullptr} {};
  ResourceHandle(const std::string& id, ResourceManager *manager)
      : resource_id{id}, resource_manager{manager} {}

  T   *get() const;
  bool is_valid() const;

  const std::string& get_id() { return resource_id; }
  T&                 operator->() const { return get(); }
  T&                 operator*() const { return *get(); };
                     operator bool() const { is_valid(); }

private:
  std::string      resource_id;
  ResourceManager *resource_manager = nullptr;
};

} // namespace Windy::Core
