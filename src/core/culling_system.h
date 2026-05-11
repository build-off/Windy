#pragma once

#include "camera.h"
#include "entity.h"
#include "mesh_component.h"
#include <vector>

namespace Windy::Core {

/// Culling, finding and setting only the elements that are visible to the
/// camera so that the rendering is mor efficient and faster
class CullingSystem {
public:
  explicit CullingSystem(Camera* cam) : camera{cam} {};
  void set_camera(Camera* cam) { camera = cam; }

  void cull_scene(const std::vector<Entity*>& all_entities) {
    visible_entitites.clear();
    if (!camera) return;

    Frustum frustum = camera->get_frustum();

    for (auto entity : all_entities) {
      // only cull entities that are visible / active
      if (!entity->is_active()) return;

      auto mesh_component = entity->get_component<MeshComponent>();
      if (!mesh_component) return;
      auto transform_component = entity->get_component<TransformComponent>();
      if (!transform_component) return;

      BoundingBox bounding_box = mesh_component->get_bounding_box();
      bounding_box.transform(transform_component->get_transform_matrix());

      if (frustum.intersects(bounding_box)) {
        visible_entitites.push_back(entity);
      }
    }
  };

  std::vector<Entity*>& get_visible_entities() { return visible_entitites; }

private:
  Camera*              camera;
  std::vector<Entity*> visible_entitites;
};

} // namespace Windy::Core
