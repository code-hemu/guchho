#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace guchho {

class ModuleResolver {
public:
    explicit ModuleResolver(std::vector<std::filesystem::path> lookup_paths);

    void add_alias(const std::string &name, const std::filesystem::path &path);
    std::filesystem::path resolve(const std::string &specifier,
                                  const std::filesystem::path &importer);

private:
    std::vector<std::filesystem::path> lookup_paths_;
    std::unordered_map<std::string, std::filesystem::path> aliases_;

    std::filesystem::path resolve_path(std::filesystem::path base);
};

}

