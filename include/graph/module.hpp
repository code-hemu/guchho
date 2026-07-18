#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace guchho {

struct Module {
    std::filesystem::path path;
    std::string source;
    std::string extension;
    std::vector<std::string> imports;
    std::vector<std::string> exports;
    bool is_entry = false;
};

}
