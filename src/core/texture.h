#pragma once

#include "resource.h"
#include "vulkan/vulkan.hpp"

namespace Windy::Core {

class Texture : public Resource {
public:
  explicit Texture(const std::string& id) : Resource{id} {};
  ~Texture() override { unload(); };

  bool do_load() override {
    // INFO: organize magic string provided by dependency
    std::string    filepath = "textures/" + get_id() + ".ktx";
    unsigned char *data = load_image_data(filepath, &width, &height, &channels);

    // maybe send in the future a notification or some message on some
    // sort of console to show that the file didnt load
    if (!data) {
      return false;
    }

    create_vulkan_image(data, width, height, channels);
    free_image_data(data);
    return Resource::load();
  }

  bool do_unload() override {
    if (is_loaded()) {
      // Handle the destruction of any extra parts, but
      // if using raii versions of vulkan dont need to
      Resource::unload();
    }
    return true;
  }

  vk::Image     get_image() const { return image; }
  vk::ImageView get_image_view() const { return imageView; }
  vk::Sampler   get_sampler() const { return sampler; }

private:
  unsigned char *load_image_data(const std::string& filepath, int *width,
                                 int *height, int *channels) {
    // Implementation using stb_image or ktx library
    // This method abstracts the details of different image format support
    // and provides a consistent interface for pixel data loading
    // ...
    return nullptr;
  };

  void free_image_data(unsigned char *data) {
    // Implementation using stb_image or ktx library
    // Ensures proper cleanup of image loader specific memory allocations
    // Different libraries may require different cleanup approaches
    // ...
  }

  // INFO: update this with the abstraction on the graphics
  // part for creating images, and common part for vulkan
  void create_vulkan_image(unsigned char *data, int width, int height,
                           int channels) {
    // Implementation to create Vulkan image, allocate memory, and upload data
    // This involves complex Vulkan operations including:
    // - Format selection based on channel count and data type
    // - Memory allocation with appropriate usage flags
    // - Image creation with optimal tiling and layout
    // - Data upload via staging buffers for efficiency
    // - Image view creation for shader access
    // - Sampler creation with appropriate filtering settings
    // ...
  }

  // INFO: move to the renderer when available, resources should not have
  // included the handles for crucial parts;
  vk::Device get_device() {
    // Get device from somewhere (e.g., singleton or parameter)
    // Production code would use dependency injection or service location
    // to provide the Vulkan device handle without tight coupling
    return vk::Device{};
  };

  vk::raii::Image        image{nullptr};
  vk::raii::DeviceMemory memory{nullptr};
  vk::DeviceSize         offset;
  vk::raii::Sampler      sampler{nullptr};
  vk::raii::ImageView    imageView{nullptr};

  int width{0};
  int height{0};
  int channels{0};
};

} // namespace Windy::Core
