#pragma once

namespace Windy::Core {

struct Frustum {};

class Camera {
public:
  Frustum get_frustum() { return frustum; };

private:
  Frustum frustum;
};

} // namespace Windy::Core
