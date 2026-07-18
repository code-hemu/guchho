#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace guchho {

struct Config {
    std::vector<std::string> entry_points;
    std::string out_dir = "dist";
    std::string out_file = "bundle.js";
    bool minify = false;
    bool source_maps = false;
    bool tree_shaking = false;
    std::string format = "esm";
    std::unordered_map<std::string, std::string> aliases;
    std::vector<std::string> external;
    std::vector<std::string> plugins;

    bool load(const std::filesystem::path &path);
    bool validate() const;
};

}
