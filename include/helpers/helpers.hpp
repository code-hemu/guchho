#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace guchho::helpers {

bool string_arrays_equal(const std::vector<std::string> &a, const std::vector<std::string> &b);
uint32_t hash_combine(uint32_t seed, uint32_t hash);
uint32_t hash_combine_string(uint32_t seed, const std::string &text);
bool is_inside_node_modules(const std::string &path);

struct GlobPart {
    std::string prefix;
    enum Wildcard { None, AllExceptSlash, AllIncludingSlash } wildcard = None;
};

std::vector<GlobPart> parse_glob_pattern(const std::string &text);

}
