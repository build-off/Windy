#include <vulkan/vulkan_core.h>
#include <vector>
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

static void glfw_error_callback(int error, const char* description) {
  WD_CORE_ERROR(description);
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

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
  vk::raii::Queue queue = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;

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
        static_cast<uint32_t>(requiredLayers.size());
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
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
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

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {},     // vk::PhysicalDeviceFeatures2 (empty for now)
            {true}, // Enable dynamic rendering from Vulkan 1.3
            {true}  // Enable extended dynamic state from the extension
        };

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
  }

  void createSurface() {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface)) {
      throw std::runtime_error("failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(instance, _surface);
  }

  void initvulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
  };
  void loop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  };

  void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
  };
};

int main(int argc, char* argv[]) {
  Windy::Log::Init();

  try {
    Renderer renderer;
    renderer.run();
  } catch (std::exception& e) {
    WD_CORE_ERROR(e.what());
    std::cerr << e.what();
    return EXIT_FAILURE;
  }
}
