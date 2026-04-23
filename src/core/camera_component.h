#pragma once

#include "component.h"
#include "entity.h"
#include "transform_component.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

namespace Windy {
class CameraComponent final : Component {
private:
  float field_of_view = 45.0f;
  float aspect_ratio  = 16.0f / 9.0f;
  float near_place    = 0.1f;
  float far_plane     = 1000.0f;

  glm::mat4 view_matrix       = glm::mat4(1.0f);
  glm::mat4 projection_matrix = glm::mat4(1.0f);
  bool      projection_dirty  = true;

public:
  void set_perspective(float fov, float aspect, float near, float far) {
    field_of_view    = fov;
    aspect_ratio     = aspect;
    near_place       = near;
    far_plane        = far;
    projection_dirty = true;
  }

  glm::mat4 get_view_matrix() const {
    auto transform = get_owner()->get_component<TransformComponent>();
    if (transform) {
      glm::vec3 position = transform->get_position();
      glm::quat rotation = transform->get_rotation();

      glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
      glm::vec3 up      = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
      return glm::lookAt(position, position + forward, up);
    }
    return glm::mat4(1.0f);
  }

  glm::mat4 GetProjectionMatrix() {
    if (projection_dirty) {
      projection_matrix = glm::perspective(glm::radians(field_of_view),
                                           aspect_ratio, near_place, far_plane);
      projection_dirty  = false;
    }
    return projection_matrix;
  }
};
} // namespace Windy
