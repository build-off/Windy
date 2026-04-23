#pragma once

#include "component.h"
#include "event.h"
#include "event_system.h"

namespace Windy::Core {
class PhysicsComponent : public Component, public EventListener {
public:
  // grab the event_system and register a new listener
  void on_init() override { get_event_system().add_listener(this); }

  // remove the listener when the component is destroyed
  ~PhysicsComponent() override { get_event_system().remove_listener(this); };

private:
  EventSystem& get_event_system() {};
};

} // namespace Windy::Core
