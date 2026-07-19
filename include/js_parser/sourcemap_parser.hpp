#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace guchho::js {

struct SourceMapEntry {
    int32_t generatedLine;
    int32_t generatedColumn;
    int32_t sourceIndex;
    int32_t originalLine;
    int32_t originalColumn;
    int32_t originalNameIndex = -1;
};

struct SourceMapData {
    std::vector<std::string> sources;
    std::vector<std::string> sourcesContent;
    std::vector<std::string> names;
    std::vector<SourceMapEntry> mappings;
    bool valid = false;
};

// VLQ decoding utilities
struct VLQResult {
    int32_t value;
    int32_t length;
    bool ok;
};

VLQResult decodeVLQ(const std::string& input, size_t offset);
VLQResult decodeVLQFromUtf16(const std::vector<uint16_t>& input, size_t offset);

// Parse a source map from a JSON string
SourceMapData parseSourceMap(std::string_view json);

} // namespace guchho::js
