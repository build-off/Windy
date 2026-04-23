#pragma once

#include "component.h"
#include "log.h"
#include "material.h"
#include "mesh.h"
#include "entity.h"
#include "transform_component.h"

namespace Windy {

class MeshComponent final : Component {
private:
  Mesh     *mesh     = nullptr;
  Material *material = nullptr;

public:
  MeshComponent(Mesh *mesh_, Material *material_)
      : mesh{mesh_}, material{material_} {};

  void      set_mesh(Mesh *m) { mesh = m; }
  void      set_material(Material *mat) { material = mat; }
  Mesh     *get_mesh() const { return mesh; }
  Material *get_material() const { return material; }

  void render() override {
    if (!mesh || !material) return;
    auto transform = get_owner()->get_component<TransformComponent>();
    if (!transform) {
      WD_CORE_ERROR("could not find transform component");
      return;
    };

    material->bind();
    material->set_uniform("", transform->get_transform_matrix());
    mesh->render();
  };
};

} // namespace Windy
