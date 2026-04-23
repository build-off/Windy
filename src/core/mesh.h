#pragma once

#include "resource.h"
#include "vertex.h"
#include "vulkan/vulkan.hpp"
#include <string>

namespace Windy::Core {

class Mesh : public Resource {
public:
  explicit Mesh(const std::string& id) : Resource{id} {};
  ~Mesh() override { unload(); };
  void render();

private:
  // INFO: add abstraction for tinygltf, but leave support for obj as well
  bool load_mesh_data(const std::string& filepath, std::vector<Vertex> vertices,
                      std::vector<uint32_t>& indices) {
    // Implementation using tinygltf or similar library
    // This method handles the complex task of:
    // - Opening and validating the mesh file format
    // - Parsing vertex attributes (positions, normals, UVs, etc.)
    // - Extracting index data that defines triangle connectivity
    // - Converting from file format to engine-specific vertex structures
    // - Performing validation to ensure data integrity
    // ...
    return true;
  };

  // INFO: replace but proper asbtractions for global use
  void CreateVertexBuffer(const std::vector<Vertex>& vertices) {
    // Implementation to create Vulkan buffer, allocate memory, and upload data
    // This involves several complex Vulkan operations:
    // - Calculating buffer size requirements based on vertex count and
    // structure
    // - Creating buffer with appropriate usage flags (vertex buffer usage)
    // - Allocating GPU memory with optimal memory type selection
    // - Uploading data via staging buffer for efficient transfer
    // - Setting up memory barriers to ensure data availability
    // ...
  }

  void CreateIndexBuffer(const std::vector<uint32_t>& indices) {
    // Implementation to create Vulkan buffer, allocate memory, and upload data
    // Similar to vertex buffer creation but optimized for index data:
    // - Buffer creation with index buffer specific usage flags
    // - Memory allocation optimized for read-heavy access patterns
    // - Efficient data transfer using appropriate staging mechanisms
    // - Index format validation (16-bit vs 32-bit indices)
    // ...
  }

  vk::Device get_device() { return vk::Device{}; }

  vk::Buffer       vertex_buffer;
  vk::DeviceMemory vertex_buffer_memory;
  vk::DeviceSize   vertex_buffer_offset;
  uint32_t         vertex_count{0};

  vk::Buffer       index_buffer;
  vk::DeviceMemory index_buffer_memory;
  vk::DeviceSize   index_buffer_offset;
  uint32_t         index_count{0};
};

} // namespace Windy::Core
