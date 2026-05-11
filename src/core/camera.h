#pragma once

#include "bounding_box.h"
namespace Windy::Core {

// INFO: impl
class Frustum {
public:
  bool intersects(BoundingBox bd_box) { return true; };
};

class Camera {
public:
  Frustum get_frustum() { return frustum; };

private:
  Frustum frustum;
};

} // namespace Windy::Core
