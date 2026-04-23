#pragma once

#include <vulkan/vulkan.hpp>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color &&
           texCoord == other.texCoord;
  }

  static vk::VertexInputBindingDescription getBindingDescription() {
    vk::VertexInputBindingDescription vertexInputBindingDescription;
    vertexInputBindingDescription.binding   = 0;
    vertexInputBindingDescription.stride    = sizeof(Vertex);
    vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

    return vertexInputBindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    vk::VertexInputAttributeDescription posDesc;
    posDesc.location = 0;
    posDesc.binding  = 0;
    posDesc.format   = vk::Format::eR32G32B32Sfloat;
    posDesc.offset   = offsetof(Vertex, pos);

    vk::VertexInputAttributeDescription colorDesc;
    colorDesc.location = 1;
    colorDesc.binding  = 0;
    colorDesc.format   = vk::Format::eR32G32B32Sfloat;
    colorDesc.offset   = offsetof(Vertex, color);

    vk::VertexInputAttributeDescription texCord;
    texCord.location = 2;
    texCord.binding  = 0;
    texCord.format   = vk::Format::eR32G32Sfloat;
    texCord.offset   = offsetof(Vertex, texCoord);
    return {posDesc, colorDesc, texCord};
  };
};

namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const {
    return ((hash<glm::vec3>()(vertex.pos) ^
             (hash<glm::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};
} // namespace std
