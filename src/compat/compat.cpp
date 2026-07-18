#include "compat/compat.hpp"
#include <sstream>

namespace guchho::compat {

Semver parse_semver(const std::string &text) {
    Semver v;
    std::string num;
    bool in_pre = false;
    for (char c : text) {
        if (c == '-') { in_pre = true; continue; }
        if (c == '.') {
            if (!in_pre && !num.empty()) {
                v.parts.push_back(std::stoi(num));
                num.clear();
            }
            continue;
        }
        if (in_pre) {
            v.pre_release += c;
        } else if (std::isdigit(c)) {
            num += c;
        }
    }
    if (!num.empty()) v.parts.push_back(std::stoi(num));
    return v;
}

int compare(const Semver &a, const Semver &b) {
    size_t max = std::max(a.parts.size(), b.parts.size());
    for (size_t i = 0; i < max; ++i) {
        int ap = i < a.parts.size() ? a.parts[i] : 0;
        int bp = i < b.parts.size() ? b.parts[i] : 0;
        if (ap != bp) return ap - bp;
    }
    if (a.pre_release.empty() && !b.pre_release.empty()) return 1;
    if (!a.pre_release.empty() && b.pre_release.empty()) return -1;
    return a.pre_release.compare(b.pre_release);
}

}
