#include "plugin/plugin_manager.hpp"
#include "plugin/plugin.hpp"

namespace guchho {

void PluginManager::register_plugin(std::unique_ptr<Plugin> plugin) {
    plugins_.push_back(std::move(plugin));
}

std::string PluginManager::run_resolve_hooks(const std::string &specifier, const std::string &importer) {
    for (auto &plugin : plugins_) {
        auto result = plugin->on_resolve(specifier, importer);
        if (!result.empty()) return result;
    }
    return specifier;
}

std::string PluginManager::run_load_hooks(const std::string &path) {
    for (auto &plugin : plugins_) {
        auto result = plugin->on_load(path);
        if (!result.empty()) return result;
    }
    return {};
}

std::string PluginManager::run_transform_hooks(const std::string &code, const std::string &path) {
    std::string result = code;
    for (auto &plugin : plugins_) {
        result = plugin->on_transform(result, path);
    }
    return result;
}

}
