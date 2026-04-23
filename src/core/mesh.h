#pragma once

#include "resource.h"
#include "vertex.h"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>

namespace Windy::Core {

class Mesh : public Resource {
public:
  explicit Mesh(const std::string& id) : Resource{id} {};
  ~Mesh() override { unload(); };

  bool do_load() override {
    std::string           filePath = "models/" + get_id() + ".gltf";
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    if (!load_mesh_data(filePath, vertices, indices)) {
      return false;
    }
    create_vertex_buffer(vertices);
    create_index_buffer(indices);
    vertex_count = static_cast<uint32_t>(vertices.size());
    index_count  = static_cast<uint32_t>(indices.size());

    return Resource::load();
  }

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
  void create_vertex_buffer(const std::vector<Vertex>& vertices) {
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

  void create_index_buffer(const std::vector<uint32_t>& indices) {
    // Implementation to create Vulkan buffer, allocate memory, and upload data
    // Similar to vertex buffer creation but optimized for index data:
    // - Buffer creation with index buffer specific usage flags
    // - Memory allocation optimized for read-heavy access patterns
    // - Efficient data transfer using appropriate staging mechanisms
    // - Index format validation (16-bit vs 32-bit indices)
    // ...
  }

  vk::Device get_device() { return vk::Device{}; }

  vk::Buffer       get_vertex_buffer() const { return vertex_buffer; }
  vk::Buffer       get_index_buffer() const { return index_buffer; }
  vk::DeviceMemory get_vertex_buffer_mem() const {
    return vertex_buffer_memory;
  }
  vk::DeviceMemory get_index_buffer_mem() const { return index_buffer_memory; }
  vk::DeviceSize   get_vertex_buffer_sz() const { return vertex_buffer_offset; }
  uint32_t         get_vertex_count() const { return vertex_count; }
  uint32_t         get_index_count() const { return index_count; }

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
