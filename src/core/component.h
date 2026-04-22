#pragma once

class Entity;
class Component {
 protected:
  Entity* owner = nullptr;
  std::string name;
  bool active = true;

 public:
  explicit Component(const std::string& component_name = "component")
      : name{component_name} {};
  virtual ~Component() = default;

  virtual void init() {};
  virtual void update(float deltaTime) {};
  virtual void render() {};
  void set_owner(Entity* entity) { owner = entity; };
  Entity* get_owner() const { return owner; };
};
