#pragma once

#include "resource.h"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>

namespace Windy::Core {

class Shader : public Resource {

public:
  Shader(const std::string& id, vk::ShaderStageFlagBits shader_stage)
      : Resource{id}, stage{shader_stage} {};
  ~Shader() override { unload(); }

  bool do_load() override {
    std::string extension;

    switch (stage) {
    case vk::ShaderStageFlagBits::eVertex:
      extension = ".vert";
      break;
    case vk::ShaderStageFlagBits::eFragment:
      extension = ".frag";
      break;
    case vk::ShaderStageFlagBits::eCompute:
      extension = ".comp";
      break;
    default:
      return false;
    }

    std::string       file_path = "shaders/" + get_id() + extension + ".spv";
    std::vector<char> shader_code;
    if (!read_file(file_path, shader_code)) {
      return false;
    }
    create_shader_module(shader_code);
    return Resource::load();
  }

  bool do_unload() override {
    if (is_loaded()) {
      Resource::unload();
    }
    return false;
  }

  vk::ShaderModule        get_shader_module() const { return shader_module; }
  vk::ShaderStageFlagBits get_stage() const { return stage; }

private:
  bool read_file(const std::string& file_path, std::vector<char>& buffer) {
    return true;
  }

  void create_shader_module(const std::vector<char>& code) {
    // Implementation to create Vulkan shader module
    // ...
  }
  vk::Device get_device() { return vk::Device{}; }

  vk::ShaderModule        shader_module;
  vk::ShaderStageFlagBits stage;
};

} // namespace Windy::Core
