#pragma once

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

  std::vector<std::pair<size_t, size_t>> semaphore_signal_wait_pairs;
  vk::raii::Device&                      device;

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
  }
};

} // namespace Windy::Core
