#pragma once

#include <cstddef>
namespace Windy {

class Entity;

class ComponentTypeIDSystem {
public:
  template <typename T>
  static size_t get_type_id() {
    static size_t type_id = next_type_id++;
    return type_id;
  };

private:
  static size_t next_type_id;
};

size_t ComponentTypeIDSystem::next_type_id = 0;

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
  template <typename T>
  static size_t get_type_id() {
    return ComponentTypeIDSystem::get_type_id<T>();
  }

  // Maybe not really a good idea to change the components name, it
  // could become a mess
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
