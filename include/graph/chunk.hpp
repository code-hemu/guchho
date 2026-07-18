#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace guchho {

struct Chunk {
    std::filesystem::path path;
    std::string code;
    std::string map;
    std::vector<std::string> entry_points;
    bool is_entry = false;
};

}
