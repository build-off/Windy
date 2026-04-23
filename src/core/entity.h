#pragma once

#include "base.h"
#include "component.h"
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace Windy::Core {

class Entity {
private:
  std::string                             name;
  bool                                    active = true;
  std::vector<std::unique_ptr<Component>> components;
  std::unordered_map<size_t, Component *> componentMap;

public:
  explicit Entity(const std::string& entityName) : name{entityName} {};
  const std::string& get_name() const { return name; }
  bool               is_active() const { return active; }
  void               set_active(bool is_active) { active = is_active; }

  void init() {
    for (auto& component : components)
      component->init();
  }

  void update(float delta_time) {
    if (!active) return;
    for (auto& component : components)
      component->update(delta_time);
  }
  void render() {
    if (!active) return;
    for (auto& component : components)
      component->render();
  }

  /// Create and assign, new components to current entity
  template <typename T, typename... Args>
  T *add_component(Args&&...args) {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from component");

    size_t type_id = Component::get_type_id<T>();
    auto   it      = componentMap.find(type_id);
    if (it != componentMap.end()) {
      return static_cast<T *>(it->second);
    }

    auto component        = CreateScope<T>(std::forward<Args>(args)...);
    T   *component_ptr    = component.get();
    componentMap[type_id] = component_ptr;
    components.push_back(std::move(component));
    return component_ptr;
  }

  template <typename T>
  T *get_component() {
    size_t type_id = Component::get_type_id<T>();
    auto   it      = componentMap.find(type_id);
    if (it != componentMap.end()) {
      return static_cast<T *>(it->second);
    }
    return nullptr;
  };

  // add better feedback for the action of removing like signaling with an event
  // even though i think it should not happen
  template <typename T>
  bool remove_component() {
    size_t type_id = Component::get_type_id<T>();
    auto   it      = componentMap.find(type_id);
    if (it != componentMap.end()) {
      Component *component_ptr = it->second;
      componentMap.erase(it);

      for (auto comp_it = components.begin(); comp_it != components.end();
           ++comp_it) {
        if (comp_it->get() == component_ptr) {
          components.erase(comp_it);
          return true;
        }
      }
    }
    return false;
  }
};

} // namespace Windy::Core
