#include "helpers/helpers.hpp"
#include <algorithm>

namespace guchho::helpers {

bool string_arrays_equal(const std::vector<std::string> &a, const std::vector<std::string> &b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

uint32_t hash_combine(uint32_t seed, uint32_t hash) {
    return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

uint32_t hash_combine_string(uint32_t seed, const std::string &text) {
    seed = hash_combine(seed, static_cast<uint32_t>(text.size()));
    for (char c : text) {
        seed = hash_combine(seed, static_cast<uint32_t>(c));
    }
    return seed;
}

bool is_inside_node_modules(const std::string &path) {
    std::string p = path;
    while (true) {
        auto slash = p.find_last_of("/\\");
        if (slash == std::string::npos) return false;
        auto dir = p.substr(0, slash);
        auto base = p.substr(slash + 1);
        if (base == "node_modules") return true;
        p = dir;
    }
}

std::vector<GlobPart> parse_glob_pattern(const std::string &text) {
    std::vector<GlobPart> pattern;
    size_t pos = 0;
    while (true) {
        auto star = text.find('*', pos);
        if (star == std::string::npos) {
            pattern.push_back({text.substr(pos), GlobPart::None});
            break;
        }
        size_t count = 1;
        while (star + count < text.size() && text[star + count] == '*') count++;
        GlobPart part;
        part.prefix = text.substr(pos, star - pos);
        part.wildcard = (count >= 2) ? GlobPart::AllIncludingSlash : GlobPart::AllExceptSlash;
        pattern.push_back(part);
        pos = star + count;
    }
    return pattern;
}

}
