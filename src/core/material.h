#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace Windy::Core {

class Material {
public:
  void bind();
  void set_uniform(std::string, glm::mat4);
};

} // namespace Windy::Core
