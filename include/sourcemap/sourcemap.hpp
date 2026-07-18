#pragma once

#include <string>
#include <vector>

namespace guchho::sourcemap {

struct Mapping {
    int32_t generated_line;    // 0-based
    int32_t generated_column;  // 0-based
    int32_t original_line;     // 0-based
    int32_t original_column;   // 0-based
    std::string source;
    std::string name;
};

struct SourceMap {
    int version = 3;
    std::string file;
    std::string source_root;
    std::vector<std::string> sources;
    std::vector<std::string> names;
    std::string mappings;

    std::string to_json() const;
};

SourceMap generate(const std::string &source, const std::string &source_file);

}
