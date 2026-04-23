#pragma once

namespace Windy {

class Entity;
class Component {
protected:
  std::string name;
  Entity     *owner  = nullptr;
  bool        active = true;

public:
  explicit Component(const std::string& component_name = "component");
  virtual ~Component() = default;

  virtual void init() {};
  virtual void update(float deltaTime);
  virtual void render();

  void    set_owner(Entity *entity) { owner = entity; }
  Entity *get_owner() const { return owner; }
};

} // namespace Windy
