#pragma once

#include "base.h"
#include "component.h"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace Windy {

class Entity {
private:
  std::string                             name;
  bool                                    active = true;
  std::vector<std::unique_ptr<Component>> components;

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
    auto component     = CreateScope<T>(std::forward(args)...);
    T   *component_ptr = component.get();
    component_ptr->set_owner(this);
    components.push_back(std::move(component));
    return component_ptr;
  }

  template <typename T>
  T *get_component() {
    for (auto& component : components) {
      // the dynamic_cast can be really slow, so change for better solution to
      // returnrning the component
      if (T *result = dynamic_cast<T *>(component.get())) {
        return result;
      }
    }
    return nullptr;
  };

  // add better feedback for the action of removing like signaling with an event
  // even though i think it should not happen
  template <typename T>
  bool remove_component() {
    for (auto it = components.begin(); it != components.end(); it++) {
      if (dynamic_cast<T *>(it->get())) {
        components.erase(it);
        return true;
      }
    }
    return false;
  }
};

} // namespace Windy
