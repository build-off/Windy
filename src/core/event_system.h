#pragma once

#include "event.h"
#include <algorithm>
#include <vector>

namespace Windy::Core {
class EventSystem {
public:
  void add_listener(EventListener *listener) { listeners.push_back(listener); }
  bool remove_listener(EventListener *listener) {
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end()) {
      listeners.erase(it);
      return true;
    }
    return false;
  }

  void dispatch_event(const Event& event) {
    for (auto listener : listeners) {
      listener->OnEvent(event);
    }
  }

private:
  std::vector<EventListener *> listeners;
};

} // namespace Windy::Core
