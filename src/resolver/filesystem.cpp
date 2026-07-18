#include "resolver/filesystem.hpp"
#include "fs/fs.hpp"

namespace guchho {
    using namespace guchho::fs;

std::filesystem::path resolve_path(const std::filesystem::path &base) {
    if (file_exists(base)) return base;
    if (file_exists(base.string() + ".js")) return base.string() + ".js";
    if (file_exists(base.string() + ".ts")) return base.string() + ".ts";
    if (file_exists(base.string() + ".json")) return base.string() + ".json";
    if (file_exists(base / "index.js")) return base / "index.js";
    if (file_exists(base / "index.ts")) return base / "index.ts";
    return base;
}

std::vector<std::filesystem::path> find_node_modules(const std::filesystem::path &start) {
    std::vector<std::filesystem::path> result;
    auto dir = std::filesystem::absolute(start);
    while (dir.has_parent_path()) {
        auto nm = dir / "node_modules";
        if (std::filesystem::exists(nm)) result.push_back(nm);
        dir = dir.parent_path();
    }
    return result;
}

}
