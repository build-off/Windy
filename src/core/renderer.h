#pragma once

#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Windy::Core {

class RenderGraph {
private:
  struct Resource {
    std::string            name;
    vk::Format             format;
    vk::Extent2D           extent;
    vk::ImageUsageFlagBits usage;
    vk::ImageLayout        initial_layout;
    vk::ImageLayout        final_layout;

    // GPU resources populated during compilation
    vk::raii::Image        image  = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    vk::raii::ImageView    view   = nullptr;

    // INFO: impl later for the assigning to work
    Resource& operator=(const Resource& other) {};
  };

  /// Render pass representation
  struct Pass {
    std::function<void(vk::raii::CommandBuffer&)> execute_func;
    std::string                                   name;
    std::vector<std::string>                      inputs;
    std::vector<std::string>                      outputs;
  };

  std::unordered_map<std::string, Resource> resources;
  std::vector<Pass>                         passes;
  std::vector<size_t>                       execution_order;

  std::vector<vk::raii::Semaphore>       semaphores;
  std::vector<std::pair<size_t, size_t>> semaphore_signal_wait_pairs;
  vk::raii::Device&                      device;

  uint32_t find_memory_type(uint32_t                type_filter,
                            vk::MemoryPropertyFlags properties) {
    return 0;
  }

public:
  explicit RenderGraph(vk::raii::Device& dev) : device{dev} {};
  void add_resource(const std::string& name, vk::Format format,
                    vk::Extent2D extent, vk::ImageUsageFlagBits usage,
                    vk::ImageLayout initial_layout,
                    vk::ImageLayout final_layout) {
    Resource resource{.name           = name,
                      .format         = format,
                      .extent         = extent,
                      .usage          = usage,
                      .initial_layout = initial_layout,
                      .final_layout   = final_layout};
    resources[name] = resource;
  };

  void add_pass(const std::string& name, const std::vector<std::string>& inputs,
                const std::vector<std::string>&               outputs,
                std::function<void(vk::raii::CommandBuffer&)> exec_func) {
    Pass pass{.execute_func = exec_func,
              .name         = name,
              .inputs       = inputs,
              .outputs      = outputs};
    passes.push_back(pass);
  }

  void compile() {
    std::vector<std::vector<size_t>> dependencies{passes.size()};
    std::vector<std::vector<size_t>> dependants{passes.size()};

    // Track which pass produces each resource (write-after-write dependencies)
    std::unordered_map<std::string, size_t> resource_writers;
    for (size_t i = 0; i < passes.size(); ++i) {
      const auto& pass = passes[i];

      // process input dependencies
      for (const auto& input : pass.inputs) {
        auto it = resource_writers.find(input);
        if (it != resource_writers.end()) {
          // found the pass that produces this input -> creating a dependency
          // link
          dependencies[i].push_back(it->second);
          dependants[it->second].push_back(i);
        }
      }

      for (const auto& output : pass.outputs)
        resource_writers[output] = i;
    }

    // Topological sort for optimal execution order
    std::vector<bool> visited(passes.size(), false);
    std::vector<bool> in_stack(passes.size(), false);

    std::function<void(size_t)> visit = [&](size_t node) {
      if (in_stack[node]) {
        // circular dependency detection
        throw std::runtime_error("Cycle detected on the render graph");
      }

      if (visited[node]) return;
      in_stack[node] = true;

      // process all the dependant passes first [post order traversal]
      for (auto dependant : dependants[node]) {
        visit(dependant);
      }

      in_stack[node] = false;
      visited[node]  = true;
      execution_order.push_back(node);
    };

    // Finish processing all unvisited nodes, to handle disconnected graph
    // components
    for (size_t i = 0; i < passes.size(); ++i) {
      if (!visited[i]) visit(i);
    }

    // Generate all the semaphores for all dependencies identified during
    // analysis
    for (size_t i = 0; i < passes.size(); ++i) {
      for (auto dep : dependencies[i]) {
        semaphores.emplace_back(device.createSemaphore({}));
        semaphore_signal_wait_pairs.emplace_back(dep, i);
      }
    }

    // Physical Resource allocation and creation
    // Transform resource descriptions into actual GPU objects
    for (auto& [name, resource] : resources) {
      vk::ImageCreateInfo image_info;
      image_info.setImageType(vk::ImageType::e2D)
          .setFormat(resource.format)
          .setExtent({resource.extent.width, resource.extent.height, 1})
          .setMipLevels(1)
          .setArrayLayers(1)
          .setSamples(vk::SampleCountFlagBits::e1)
          .setTiling(vk::ImageTiling::eOptimal)
          .setUsage(resource.usage)
          .setSharingMode(vk::SharingMode::eExclusive)
          .setInitialLayout(vk::ImageLayout::eUndefined);

      resource.image = device.createImage(image_info);

      vk::MemoryRequirements mem_requirements =
          resource.image.getMemoryRequirements();
      vk::MemoryAllocateInfo alloc_info;
      alloc_info.setAllocationSize(mem_requirements.size)
          .setMemoryTypeIndex(
              find_memory_type(mem_requirements.memoryTypeBits,
                               vk::MemoryPropertyFlagBits::eDeviceLocal));

      resource.memory = device.allocateMemory(alloc_info);
      resource.image.bindMemory(*resource.memory, 0);

      vk::ImageViewCreateInfo view_info;
      view_info.setImage(*resource.image)
          .setViewType(vk::ImageViewType::e2D)
          .setFormat(resource.format)
          .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

      resource.view = device.createImageView(view_info);
    }
  }

  Resource* get_resource(const std::string& name) {
    auto it = resources.find(name);
    return (it != resources.end()) ? &it->second : nullptr;
  }

  // rendergraph execution engine [automatic syncronization]
  void execute(vk::raii::CommandBuffer& command_buffer, vk::Queue queue) {
    std::vector<vk::CommandBuffer>      cmd_buffers;
    std::vector<vk::Semaphore>          wait_semaphores;
    std::vector<vk::PipelineStageFlags> wait_stages;
    std::vector<vk::Semaphore>          signal_semaphores;

    // execute each pass in the computed dependency-safe order
    for (auto pass_inx : execution_order) {
      const auto& pass = passes[pass_inx];
    }
  }
};

} // namespace Windy::Core
