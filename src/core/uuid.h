#pragma once

namespace Windy::Core {

class uuid {
public:
  uuid();
  uuid(uint64_t uuid);
  uuid(const uuid&) = default;

  operator uint64_t() const { return m_uuid; }

private:
  uint64_t m_uuid;
};

} // namespace Windy::Core

namespace std {
template <typename T>
struct hash;
template <>
struct hash<Windy::Core::uuid> {
  size_t operator()(const Windy::Core::uuid& uuid) const {
    return (uint64_t)uuid;
  }
};
} // namespace std
