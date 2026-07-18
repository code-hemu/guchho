#pragma once

#include <memory>
#include <string>
#include <vector>

namespace guchho {

struct Plugin;

class PluginManager {
public:
    void register_plugin(std::unique_ptr<Plugin> plugin);
    std::string run_resolve_hooks(const std::string &specifier, const std::string &importer);
    std::string run_load_hooks(const std::string &path);
    std::string run_transform_hooks(const std::string &code, const std::string &path);

private:
    std::vector<std::unique_ptr<Plugin>> plugins_;
};

}
