#include "sourcemap/sourcemap.hpp"
#include "logger/logger.hpp"
#include <sstream>

namespace guchho::sourcemap {

std::string SourceMap::to_json() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"version\": " << version << ",\n";
    ss << "  \"file\": \"" << file << "\",\n";
    ss << "  \"sourceRoot\": \"" << source_root << "\",\n";
    ss << "  \"sources\": [";
    for (size_t i = 0; i < sources.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"" << sources[i] << "\"";
    }
    ss << "],\n";
    ss << "  \"names\": [";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\"" << names[i] << "\"";
    }
    ss << "],\n";
    ss << "  \"mappings\": \"" << mappings << "\"\n";
    ss << "}";
    return ss.str();
}

SourceMap generate(const std::string &source, const std::string &source_file) {
    SourceMap map;
    map.file = source_file;
    map.sources.push_back(source_file);

    std::vector<int> line_starts;
    line_starts.push_back(0);
    for (size_t i = 0; i < source.size(); ++i) {
        if (source[i] == '\n') {
            line_starts.push_back(static_cast<int>(i) + 1);
        }
    }

    log().info("Generated source map for " + source_file);
    return map;
}

}
