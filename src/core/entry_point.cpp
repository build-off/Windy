#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>
#include <fstream>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "vulkan/vulkan.hpp"

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const std::vector<char const*> validationlayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NODEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "log.h"

class Renderer {
 public:
  void run() {
    initwindow();
    initvulkan();
    loop();
    cleanup();
  };

  ~Renderer() {};

 private:
  constexpr static uint32_t WIDTH = 800;
  constexpr static uint32_t HEIGHT = 600;

  GLFWwindow* window;
  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  vk::PhysicalDeviceFeatures deviceFeatures;
  uint32_t queueIndex = ~0;
  vk::raii::Queue queue = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::Extent2D swapChainExtent;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;
  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffers;

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  vk::raii::DescriptorPool descriptorPool = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;

  // Syncronization
  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inflightfences;

  vk::raii::Image textureImage = nullptr;
  vk::raii::DeviceMemory textureImageMemory = nullptr;

  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
  uint32_t frameInx = 0;
  bool framebufferResize = false;

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
      vk::VertexInputBindingDescription vertexInputBindingDescription;
      vertexInputBindingDescription.binding = 0;
      vertexInputBindingDescription.stride = sizeof(Vertex);
      vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

      return vertexInputBindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
      vk::VertexInputAttributeDescription posDesc;
      posDesc.location = 0;
      posDesc.binding = 0;
      posDesc.format = vk::Format::eR32G32Sfloat;
      posDesc.offset = offsetof(Vertex, pos);
      vk::VertexInputAttributeDescription colorDesc;
      colorDesc.location = 1;
      colorDesc.binding = 0;
      colorDesc.format = vk::Format::eR32G32B32Sfloat;
      colorDesc.offset = offsetof(Vertex, color);
      return {posDesc, colorDesc};
    };
  };

  // combining the vertex data like its position and color, is called
  // interleaving vertex attributes
  const std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

  struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
  };

  static void framebufferResizeCallback(GLFWwindow* window, int width,
                                        int height) {
    auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResize = true;
  }

  static void glfw_error_callback(int error, const char* description) {
    WD_CORE_ERROR(description);
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
  }

  static std::vector<char> read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    return buffer;
  }

  std::vector<const char*> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
  }

  void createInstance() {
    constexpr vk::ApplicationInfo appInfo{"Windy", VK_MAKE_VERSION(1, 0, 0),
                                          "No engine", VK_MAKE_VERSION(1, 0, 0),
                                          vk::ApiVersion14};
    // Get the required instance extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto extensionProperties = context.enumerateInstanceExtensionProperties();

    // Check if the current vulkan implementation supports the extensions
    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
      if (std::ranges::none_of(
              extensionProperties, [glfwExtension = glfwExtensions[i]](
                                       auto const& extensionProperty) {
                return strcmp(extensionProperty.extensionName, glfwExtension) ==
                       0;
              })) {
        throw std::runtime_error("Required GLFW extension not supported: " +
                                 std::string(glfwExtensions[i]));
      }
    }

    // Assign the validation layers, if enabled
    std::vector<char const*> requiredLayers;
    if (enableValidationLayers) {
      requiredLayers.assign(validationlayers.begin(), validationlayers.end());
    }

    // Check if the required layers are supported by the vulkan implentation
    auto layersProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
        requiredLayers, [&layersProperties](auto const& requiredLayer) {
          return std::ranges::none_of(
              layersProperties, [requiredLayer](auto const& layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
              });
        });

    if (unsupportedLayerIt != requiredLayers.end())
      throw std::runtime_error("Required layer not supported: " +
                               std::string(*unsupportedLayerIt));

    // Required instance extensions [ GLFW ]
    auto requiredExtensions = getRequiredInstanceExtensions();

    // Check if the required extensions by glfw are available by the current
    // vulkan implemenetation
    auto unsupportedPropertyIt = std::ranges::find_if(
        requiredExtensions,
        [&extensionProperties](auto const& requiredExtension) {
          return std::ranges::none_of(
              extensionProperties,
              [requiredExtension](auto const& extensionProperty) {
                return strcmp(extensionProperty.extensionName,
                              requiredExtension) == 0;
              });
        });

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    createInfo.ppEnabledLayerNames = requiredLayers.data();

    instance = vk::raii::Instance(context, createInfo);
  }

  std::vector<const char*> requiredDeviceExtension = {
      vk::KHRSwapchainExtensionName};

  bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) {
    // validate for vulkan1.3, queue family suport with graphical
    // operations, support for all the extensions and features
    bool supportsVulkan1_3 =
        physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics =
        std::ranges::any_of(queueFamilies, [](auto const& qfp) {
          return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        });

    auto availableDeviceExtensions =
        physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension,
        [&availableDeviceExtensions](auto const& requiredDeviceExtension) {
          return std::ranges::any_of(
              availableDeviceExtensions,
              [requiredDeviceExtension](auto const& availableDeviceExtension) {
                return strcmp(availableDeviceExtension.extensionName,
                              requiredDeviceExtension) == 0;
              });
        });

    auto features = physicalDevice.template getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures =
        features.template get<vk::PhysicalDeviceVulkan13Features>()
            .dynamicRendering &&
        features
            .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
            .extendedDynamicState;

    return supportsVulkan1_3 && supportsGraphics &&
           supportsAllRequiredExtensions && supportsRequiredFeatures;
  }

  // Select the GPU to use, although multiple can be used
  // at the same time
  //
  // more dificult seleccion proccess can be made, like with a score
  // depending on the available features, or send the options and get
  // a selection from the user
  void pickPhysicalDevice() {
    auto physicalDevices = instance.enumeratePhysicalDevices();
    auto const devIter =
        std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) {
          return isDeviceSuitable(physicalDevice);
        });

    if (devIter == physicalDevices.end()) {
      throw std::runtime_error("failed to find a suitable GPU!");
    }
    physicalDevice = *devIter;
  }

  void initwindow() {
    WD_CORE_INFO("Starting window");
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Windy", nullptr, nullptr);
    if (!window) {
      throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  }

  void createLogicalDevice() {
    // get a queue with graphics capabilities
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice.getQueueFamilyProperties();

    uint32_t queueInx = ~0;
    for (uint32_t qfpInx = 0; qfpInx < queueFamilyProperties.size(); ++qfpInx) {
      if ((queueFamilyProperties[qfpInx].queueFlags &
           vk::QueueFlagBits::eGraphics) &&
          physicalDevice.getSurfaceSupportKHR(qfpInx, *surface)) {
        queueInx = qfpInx;
        break;
      }
    }

    if (queueInx == ~0) {
      throw std::runtime_error(
          "Could not find a queue for graphics and present -> terminating");
    }
    // if there is a valid index for the retreived family assign it
    queueIndex = queueInx;

    vk::StructureChain<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain;
    // To allow DrawParameters on the shader
    auto& v11 = featureChain.get<vk::PhysicalDeviceVulkan11Features>();
    v11.shaderDrawParameters = VK_TRUE;

    // vk::PhysicalDeviceFeatures2 (empty for now)
    auto& features2 = featureChain.get<vk::PhysicalDeviceFeatures2>();
    // Enable dynamic rendering from Vulkan 1.3
    auto& v13 = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
    v13.dynamicRendering = VK_TRUE;
    v13.synchronization2 = VK_TRUE;
    // Enable extended dynamic state from the extension
    auto& ext =
        featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    ext.extendedDynamicState = VK_TRUE;

    float queue_priority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.queueFamilyIndex = queueInx;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queue_priority;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredDeviceExtension.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data();

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    queue = vk::raii::Queue(device, queueInx, 0);

    // Swap chain

    // details of swap chain support
    // - basic swapchain support (number of images, width and height of them)
    // - surface formats (pixel formats, color space)
    // - available presentation modes
    auto surfaceCapabilities =
        physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    std::vector<vk::SurfaceFormatKHR> availableFormats =
        physicalDevice.getSurfaceFormatsKHR(surface);
    std::vector<vk::PresentModeKHR> availablePresentModes =
        physicalDevice.getSurfacePresentModesKHR(surface);

    // Surface format - color depth
    // Presentation mode - conditions for swapping images to the screen
    // Swap extent - resolution of images in swapchain
  }

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      std::vector<vk::SurfaceFormatKHR> const& availableFormats) {
    const auto formatIt =
        std::ranges::find_if(availableFormats, [](const auto& format) {
          return format.format == vk::Format::eB8G8R8A8Srgb &&
                 format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
  }

  vk::PresentModeKHR chooseSwapPresentMode(
      std::vector<vk::PresentModeKHR> const& availablePresentModes) {
    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {
      return presentMode == vk::PresentModeKHR::eFifo;
    }));
    return std::ranges::any_of(availablePresentModes,
                               [](const vk::PresentModeKHR value) {
                                 return vk::PresentModeKHR::eMailbox == value;
                               })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D chooseSwapExtent(
      vk::SurfaceCapabilitiesKHR const& capabilities) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                                 capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)};
  }

  static uint32_t chooseSwapMinImageCount(
      vk::SurfaceCapabilitiesKHR const& surfaceCapabilitites) {
    auto minImageCount = std::max(3u, surfaceCapabilitites.minImageCount);
    if ((0 < surfaceCapabilitites.maxImageCount) &&
        (surfaceCapabilitites.maxImageCount < minImageCount)) {
      minImageCount = surfaceCapabilitites.maxImageCount;
    }
    return minImageCount;
  }

  void createSwapChain() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilitites =
        physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainExtent = chooseSwapExtent(surfaceCapabilitites);
    uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilitites);

    // Surface format
    std::vector<vk::SurfaceFormatKHR> availableFormats =
        physicalDevice.getSurfaceFormatsKHR(*surface);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

    uint32_t imageCount = surfaceCapabilitites.minImageCount + 1;
    std::vector<vk::PresentModeKHR> availablePresentModes =
        physicalDevice.getSurfacePresentModesKHR(*surface);

    // CreateInfo swapchain struct
    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.surface = *surface;
    swapChainCreateInfo.minImageCount = minImageCount;
    swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapChainCreateInfo.preTransform = surfaceCapabilitites.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode =
        chooseSwapPresentMode(availablePresentModes);
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr;

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
  }

  void createSurface() {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface)) {
      throw std::runtime_error("failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(instance, _surface);
  }

  void createImageViews() {
    assert(swapChainImageViews.empty());
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = swapChainSurfaceFormat.format;
    imageViewCreateInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0,
                                            1, 0, 1};
    imageViewCreateInfo.components = {
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};

    for (auto& image : swapChainImages) {
      imageViewCreateInfo.image = image;
      swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
  }

  // Shader modules
  [[nodiscard]] vk::raii::ShaderModule createShaderModule(
      const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size() * sizeof(char);
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    vk::raii::ShaderModule shaderModule{device, createInfo};
    return shaderModule;
  }

  void createGraphicsPipeline() {
    vk::raii::ShaderModule shaderModule =
        createShaderModule(read_file("shaders/slang.spv"));
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = shaderModule;
    vertShaderStageInfo.pName = "vertMain";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = shaderModule;
    fragShaderStageInfo.pName = "fragMain";

    // Pipeline creation steps
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                        fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

    // Viewports and scissors
    // Viewport -> the region of the framebuffer that the output will be
    // rendered to
    vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                          static_cast<float>(swapChainExtent.height)};
    vk::Rect2D scissor{vk::Offset2D{0, 0}, swapChainExtent};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                   vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.pScissors = &scissor;

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = vk::False;
    rasterizer.rasterizerDiscardEnable = vk::False;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = vk::False;
    rasterizer.lineWidth = 1.0f;

    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.sampleShadingEnable = vk::False;

    // Color blending -> using bitwise operations
    vk::PipelineColorBlendAttachmentState colorBlendAttatchment;
    colorBlendAttatchment.blendEnable = vk::False;
    colorBlendAttatchment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttatchment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttatchment.dstColorBlendFactor =
        vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttatchment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttatchment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttatchment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttatchment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOpEnable = vk::False;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttatchment;

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &*descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    // Render passes
    // Dynamic rendering -> we need to pass the format of the attatchments, for
    // the rendering

    vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                       vk::PipelineRenderingCreateInfo>
        pipelineCreteInfoChain{};
    // Graphics Pipeline
    auto& graphicsPipelineCreateInfo =
        pipelineCreteInfoChain.get<vk::GraphicsPipelineCreateInfo>();
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = nullptr;

    // Pipeline Rendering
    auto& pipelineRenderingCreateInfo =
        pipelineCreteInfoChain.get<vk::PipelineRenderingCreateInfo>();
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats =
        &swapChainSurfaceFormat.format;

    graphicsPipeline = vk::raii::Pipeline(
        device, nullptr,
        pipelineCreteInfoChain.get<vk::GraphicsPipelineCreateInfo>());
  }

  void createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueIndex;
    commandPool = vk::raii::CommandPool(device, poolInfo);
  }

  void createCommandBuffers() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
  }

  void recordCommandBuffer(uint32_t imageIndex) {
    commandBuffers[frameInx].begin({});
    transition_image_layout(imageIndex, vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eColorAttachmentOptimal, {},
                            vk::AccessFlagBits2::eColorAttachmentWrite,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attInfo;
    attInfo.imageView = swapChainImageViews[imageIndex];
    attInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attInfo.loadOp = vk::AttachmentLoadOp::eClear;
    attInfo.storeOp = vk::AttachmentStoreOp::eStore;
    attInfo.clearValue = clearColor;

    vk::RenderingInfo renderingInfo;
    vk::Rect2D render_area;
    render_area.offset = vk::Offset2D{0, 0};
    render_area.extent = swapChainExtent;
    renderingInfo.renderArea = render_area;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &attInfo;
    commandBuffers[frameInx].beginRendering(renderingInfo);
    commandBuffers[frameInx].bindPipeline(vk::PipelineBindPoint::eGraphics,
                                          *graphicsPipeline);
    commandBuffers[frameInx].bindVertexBuffers(0, *vertexBuffer, {0});
    commandBuffers[frameInx].bindIndexBuffer(*indexBuffer, 0,
                                             vk::IndexType::eUint16);
    commandBuffers[frameInx].setViewport(
        0,
        vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                     static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffers[frameInx].setScissor(
        0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

    // binding the descriptor sets
    commandBuffers[frameInx].bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
        *descriptorSets[frameInx], nullptr);

    commandBuffers[frameInx].drawIndexed(indices.size(), 1, 0, 0, 0);
    commandBuffers[frameInx].endRendering();

    transition_image_layout(imageIndex,
                            vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::ePresentSrcKHR,
                            vk::AccessFlagBits2::eColorAttachmentWrite, {},
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eBottomOfPipe);
    commandBuffers[frameInx].end();
  }

  void transition_image_layout(uint32_t imageInx, vk::ImageLayout old_layout,
                               vk::ImageLayout new_layout,
                               vk::AccessFlags2 src_access_mask,
                               vk::AccessFlags2 dst_access_mask,
                               vk::PipelineStageFlags2 src_stage_mask,
                               vk::PipelineStageFlags2 dst_stage_mask) {
    vk::ImageMemoryBarrier2 barrier;
    barrier.srcStageMask = src_stage_mask;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstStageMask = dst_stage_mask;
    barrier.dstAccessMask = dst_access_mask;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapChainImages[imageInx];
    vk::ImageSubresourceRange sub_resource_range;
    sub_resource_range.aspectMask = vk::ImageAspectFlagBits::eColor;
    sub_resource_range.baseMipLevel = 0;
    sub_resource_range.levelCount = 1;
    sub_resource_range.baseArrayLayer = 0;
    sub_resource_range.layerCount = 1;
    barrier.subresourceRange = sub_resource_range;

    vk::DependencyInfo dependency_info;
    dependency_info.dependencyFlags = {};
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    commandBuffers[frameInx].pipelineBarrier2(dependency_info);
  }

  void createSyncObjects() {
    assert(presentCompleteSemaphores.empty() &&
           renderFinishedSemaphores.empty() && inflightfences.empty());

    renderFinishedSemaphores.reserve(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); ++i) {
      renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo{});
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo{});
      vk::FenceCreateInfo fenceCreateInfo;
      fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
      inflightfences.emplace_back(device, fenceCreateInfo);
    }
  }

  // getting the correct memory to assign to buffers
  uint32_t find_memory_type(uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties =
        physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
      if ((typeFilter & (1 << i)) &&
          (memProperties.memoryTypes[i].propertyFlags & properties) ==
              properties) {
        return i;
      }
    }
    throw std::runtime_error("failed to find suitable memory type!");
  }

  vk::raii::CommandBuffer beginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffer commandBuffer =
        std::move(device.allocateCommandBuffers(allocInfo).front());
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);
    return commandBuffer;
  }

  void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer) {
    commandBuffer.end();
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;
    queue.submit(submitInfo, nullptr);
    queue.waitIdle();
  }

  void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer,
                  vk::DeviceSize size) {
    vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
    commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer,
                                 vk::BufferCopy(0, 0, size));
    endSingleTimeCommands(commandCopyBuffer);
  }

  void transitionImageLayout(const vk::raii::Image& image,
                             vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout) {
    auto commandBuffer = beginSingleTimeCommands();
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.image = image;
    vk::ImageSubresourceRange subRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0,
                                       1};
    barrier.subresourceRange = subRange;
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {},
                                  nullptr, barrier);

    endSingleTimeCommands(commandBuffer);
  }

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties,
                    vk::raii::Buffer& buffer,
                    vk::raii::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    buffer = vk::raii::Buffer(device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        find_memory_type(memRequirements.memoryTypeBits, properties);
    bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
  }

  void createUniformBuffers() {
    uniformBuffers.clear();
    uniformBuffersMemory.clear();
    uniformBuffersMapped.clear();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
      vk::raii::Buffer buffer({});
      vk::raii::DeviceMemory bufferMem({});
      createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                   vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent,
                   buffer, bufferMem);
      uniformBuffers.emplace_back(std::move(buffer));
      uniformBuffersMemory.emplace_back(std::move(bufferMem));
      uniformBuffersMapped.emplace_back(
          uniformBuffersMemory[i].mapMemory(0, bufferSize));
    }
  }

  void createIndexBuffer() {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible |
                     vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, indices.data(), (size_t)bufferSize);
    stagingBufferMemory.unmapMemory();

    createBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst |
                     vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer,
                 indexBufferMemory);
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
  }

  void createVertexBuffer() {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    vk::BufferCreateInfo stagingInfo{};
    stagingInfo.size = bufferSize;
    stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingInfo.sharingMode = vk::SharingMode::eExclusive;

    vk::raii::Buffer stagingBuffer(device, stagingInfo);
    vk::MemoryRequirements memRequirementsStaging =
        stagingBuffer.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfoStaging{};
    memoryAllocateInfoStaging.allocationSize = memRequirementsStaging.size;
    memoryAllocateInfoStaging.memoryTypeIndex =
        find_memory_type(memRequirementsStaging.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::raii::DeviceMemory stagingBufferMemory(device,
                                               memoryAllocateInfoStaging);
    stagingBuffer.bindMemory(stagingBufferMemory, 0);
    void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
    memcpy(dataStaging, vertices.data(), stagingInfo.size);
    stagingBufferMemory.unmapMemory();

    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = bufferSize;
    bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer |
                       vk::BufferUsageFlagBits::eTransferDst;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    vertexBuffer = vk::raii::Buffer(device, bufferInfo);

    vk::MemoryRequirements memRequirements =
        vertexBuffer.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.allocationSize = memRequirements.size;
    memoryAllocateInfo.memoryTypeIndex =
        find_memory_type(memRequirementsStaging.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eDeviceLocal);
    vertexBufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);
    vertexBuffer.bindMemory(*vertexBufferMemory, 0);
    copyBuffer(stagingBuffer, vertexBuffer, stagingInfo.size);
  }

  void createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, 1,
        vk::ShaderStageFlagBits::eVertex, nullptr);
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
  }

  void createDescriptorPool() {
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer,
                                    MAX_FRAMES_IN_FLIGHT);
    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
  }

  void createDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                                 *descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.clear();
    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vk::DescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      vk::WriteDescriptorSet descriptorWrite{};
      descriptorWrite.dstSet = descriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
      descriptorWrite.pBufferInfo = &bufferInfo;
      device.updateDescriptorSets(descriptorWrite, {});
    }
  }

  void createImage(uint32_t width, uint32_t height, vk::Format format,
                   vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                   vk::MemoryPropertyFlags properties, vk::raii::Image& image,
                   vk::raii::DeviceMemory& imageMemory) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = format;
    vk::Extent3D extenxt{width, height, 1};
    imageInfo.extent = extenxt;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    image = vk::raii::Image(device, imageInfo);
    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        find_memory_type(memRequirements.memoryTypeBits, properties);
    imageMemory = vk::raii::DeviceMemory(device, allocInfo);
    image.bindMemory(imageMemory, 0);
  }

  void createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) throw std::runtime_error("failed to load texture image!");

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});

    createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible |
                     vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);
    void* data = stagingBufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();
    stbi_image_free(pixels);

    vk::raii::Image textureImageTemp({});
    vk::raii::DeviceMemory textureImageMemoryTemp({});
    createImage(
        texWidth, texHeight, vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal, textureImageTemp,
        textureImageMemoryTemp);
  }

  void initvulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createTextureImage();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
  };
  void loop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawframe();
    }
    device.waitIdle();
  };

  void cleanupSwapChain() {
    swapChainImageViews.clear();
    swapChain = nullptr;
  }

  void recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }

    device.waitIdle();
    cleanupSwapChain();
    createSwapChain();
    createImageViews();
  }

  void updateUniformBuffer(uint32_t currentImage) {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     current_time - start_time)
                     .count();

    UniformBufferObject ubo{};
    ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                       glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),
                                static_cast<float>(swapChainExtent.width) /
                                    static_cast<float>(swapChainExtent.height),
                                0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
  }

  void drawframe() {
    auto fenceres =
        device.waitForFences(*inflightfences[frameInx], vk::True, UINT64_MAX);
    if (fenceres != vk::Result::eSuccess) {
      throw std::runtime_error("failed to wait for fence!");
    }
    auto [result, imageIndex] = swapChain.acquireNextImage(
        UINT64_MAX, *presentCompleteSemaphores[frameInx], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
      recreateSwapChain();
      return;
    }

    if (result != vk::Result::eSuccess &&
        result != vk::Result::eSuboptimalKHR) {
      assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
      throw std::runtime_error("failed to acquire swapchain image");
    }

    updateUniformBuffer(frameInx);
    // only reset if work is going to be submitted, to avoid deadlock
    device.resetFences(*inflightfences[frameInx]);

    commandBuffers[frameInx].reset();
    recordCommandBuffer(imageIndex);

    auto& render_finished = renderFinishedSemaphores[imageIndex];

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &*presentCompleteSemaphores[frameInx];
    submitInfo.pWaitDstStageMask = &waitDestinationStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffers[frameInx];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &*render_finished;

    queue.submit(submitInfo, *inflightfences[frameInx]);
    vk::PresentInfoKHR presentInfoKHR;
    presentInfoKHR.waitSemaphoreCount = 1;
    presentInfoKHR.pWaitSemaphores = &*render_finished;
    presentInfoKHR.swapchainCount = 1;
    presentInfoKHR.pSwapchains = &*swapChain;
    presentInfoKHR.pImageIndices = &imageIndex;
    presentInfoKHR.pResults = nullptr;
    result = queue.presentKHR(presentInfoKHR);

    if ((result == vk::Result::eSuboptimalKHR) ||
        (result == vk::Result::eErrorOutOfDateKHR || framebufferResize)) {
      framebufferResize = false;
      recreateSwapChain();
    } else {
      // There are no other success codes than eSuccess; on any error code,
      // presentKHR already threw an exception.
      assert(result == vk::Result::eSuccess);
    }
    frameInx = (frameInx + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void cleanup() {
    cleanupSwapChain();
    glfwDestroyWindow(window);
  };
};

int main(int argc, char* argv[]) {
  Windy::Log::Init();
  try {
    {
      Renderer renderer;
      renderer.run();
    }
    glfwTerminate();
  } catch (std::exception& e) {
    WD_CORE_ERROR(e.what());
    std::cerr << e.what();
    return EXIT_FAILURE;
  }
}
