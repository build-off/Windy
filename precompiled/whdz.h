#pragma once

#include <cstdint>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#if defined(__intellisense__) || !defined(use_cpp20_modules)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
