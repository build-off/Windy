#pragma once

#include "entity.h"

namespace Windy {

class Event {
public:
  virtual ~Event() = default;
};

class EventListener {
public:
  virtual ~EventListener()                 = default;
  virtual void OnEvent(const Event& event) = 0;
};

// Example implementation of an event
class CollisionEvent : public Event {
public:
  CollisionEvent(Entity *e1, Entity *e2) : ent1{e1}, ent2{e2} {};
  Entity *get_ent1() const { return ent1; }
  Entity *get_ent2() const { return ent2; }

private:
  Entity *ent1;
  Entity *ent2;
};

} // namespace Windy
