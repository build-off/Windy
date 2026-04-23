#pragma once

namespace Windy {

class Entity;
class Component {
public:
  enum class State {
    Uninitialized,
    Initializing,
    Active,
    Destroying,
    Destroyed
  };

protected:
  std::string name;
  bool        active = true;

  virtual void on_init() {};
  virtual void on_destroy() {};
  virtual void update(float delta_time) {};
  virtual void render() {};

  friend class Entity;

private:
  Entity *owner = nullptr;
  State   state = State::Uninitialized;

public:
  explicit Component(const std::string& component_name = "component");
  virtual ~Component() {
    if (state != State::Destroyed) {
      on_destroy();
      state = State::Destroyed;
    }
  };

  void init() {
    if (state == State::Uninitialized) {
      state = State::Initializing;
      on_init();
      state = State::Active;
    }
  };

  void destroy() {
    if (state == State::Active) {
      state = State::Destroying;
      on_destroy();
      state = State::Destroyed;
    }
  }

  void    set_owner(Entity *entity) { owner = entity; }
  Entity *get_owner() const { return owner; }
};

} // namespace Windy
