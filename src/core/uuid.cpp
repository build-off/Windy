#include "uuid.h"
#include <random>

namespace Windy::Core {
static std::random_device                      s_random_device;
static std::mt19937_64                         s_engine(s_random_device());
static std::uniform_int_distribution<uint64_t> s_unfirom_distribution;

uuid::uuid() : m_uuid{s_unfirom_distribution(s_engine)} {
}

uuid::uuid(uint64_t uuid) : m_uuid{uuid} {
}

} // namespace Windy::Core
