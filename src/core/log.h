#pragma once

#include <whdz.h>
#include <glm/detail/type_quat.hpp>
#include "base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Windy {
class Log {
 public:
  static void Init();
  static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
  static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

 private:
  static Ref<spdlog::logger> s_CoreLogger;
  static Ref<spdlog::logger> s_ClientLogger;
};
} // namespace Windy

// Printing for physics types
template <typename ostream, glm::length_t L, typename T, glm::qualifier Q>
inline ostream& operator<<(ostream& os, const glm::vec<L, T, Q>& vector) {
  return os << glm::to_string(vector);
}
template <typename ostream, glm::length_t C, glm::length_t R, typename T,
          glm::qualifier Q>
inline ostream& operator<<(ostream& os, const glm::mat<C, R, T, Q>& matrix) {
  return os << glm::to_string(matrix);
}
template <typename ostream, typename T, glm::qualifier Q>
inline ostream& operator<<(ostream& os, glm::qua<T, Q> quaternion) {
  return os << glm::to_string(quaternion);
}

#define WD_CORE_TRACE(...) ::Windy::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define WD_CORE_INFO(...) ::Windy::Log::GetCoreLogger()->info(__VA_ARGS__)
#define WD_CORE_WARN(...) ::Windy::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define WD_CORE_ERROR(...) ::Windy::Log::GetCoreLogger()->error(__VA_ARGS__)
#define WD_CORE_CRITICAL(...) \
  ::Windy::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define WD_TRACE(...) ::Windy::Log::GetClientLogger()->trace(__VA_ARGS__)
#define WD_INFO(...) ::Windy::Log::GetClientLogger()->info(__VA_ARGS__)
#define WD_WARN(...) ::Windy::Log::GetClientLogger()->warn(__VA_ARGS__)
#define WD_ERROR(...) ::Windy::Log::GetClientLogger()->error(__VA_ARGS__)
#define WD_CRITICAL(...) ::Windy::Log::GetClientLogger()->critical(__VA_ARGS__)
