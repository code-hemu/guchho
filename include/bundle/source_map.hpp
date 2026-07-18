#pragma once

#include <string>
#include <vector>

namespace guchho {

struct SourceMapEntry {
    int generated_line;
    int generated_column;
    int original_line;
    int original_column;
    std::string source;
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

SourceMap generate_source_map(const std::string &source, const std::string &source_file);

}
