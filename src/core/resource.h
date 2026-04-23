#pragma once

namespace Windy::Core {

class Resource {
public:
  explicit Resource(const std::string& id) : resource_id{id} {};
  virtual ~Resource() = default;

  const std::string& get_id() const { return resource_id; }
  bool               is_loaded() const { return loaded; }

  bool load() {
    loaded = do_load();
    return loaded;
  }

  void unload() {
    do_unload();
    loaded = false;
  }

protected:
  virtual bool do_load()   = 0;
  virtual bool do_unload() = 0;

private:
  std::string resource_id;
  bool        loaded = false;
};

} // namespace Windy::Core
