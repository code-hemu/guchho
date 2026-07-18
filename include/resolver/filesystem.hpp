#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace guchho {

std::filesystem::path resolve_path(const std::filesystem::path &base);
std::vector<std::filesystem::path> find_node_modules(const std::filesystem::path &start);

}
