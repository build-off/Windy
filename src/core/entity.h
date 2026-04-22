#pragma once

#include "component.h"
#include <memory>
#include <string>

class Entity {
 private:
  std::string name;
  bool active = true;
  std::vector<std::unique_ptr<Component>> components;

 public:
  explicit Entity(const std::string& entityName) : name{entityName} {};
  const std::string& get_name() const { return name; }
  bool is_active() const { return active; }
  void set_active(bool is_active) { active = is_active; }

  void init() {
    for (auto& component : components) component->init();
  }

  void update(float delta_time) {
    if (!active) return;
    for (auto& component : components) component->update(delta_time);
  }
};
