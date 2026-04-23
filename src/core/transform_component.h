#pragma once
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include "component.h"

namespace Windy {

class TransformComponent final : Component {
private:
  glm::vec3 scale    = glm::vec3(1.0f);
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

  // cached matrix transformations
  mutable glm::mat4 transform_matrix = glm::mat4(1.0f);
  mutable bool      transform_dirty  = true;

public:
  void set_position() {};
  void set_rotation() {};
  void set_scale() {};

  const glm::vec3& get_position() const { return position; }
  const glm::quat& get_rotation() const { return rotation; }
  const glm::vec3& get_scale() const { return scale; }

  glm::mat4 get_transform_matrix() {
    if (transform_dirty) {
      glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 rotation_matrix    = glm::mat4_cast(rotation);
      glm::mat4 scale_matrix       = glm::scale(glm::mat4(1.0f), scale);
      transform_matrix = translation_matrix * rotation_matrix * scale_matrix;
      transform_dirty  = false;
    }
    return transform_matrix;
  }
};

} // namespace Windy
