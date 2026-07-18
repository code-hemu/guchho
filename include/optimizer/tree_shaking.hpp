#pragma once

#include <string>
#include <unordered_set>

namespace guchho {

class TreeShaker {
public:
    void mark_used(const std::string &symbol);
    bool is_used(const std::string &symbol) const;
    std::string shake(const std::string &source);
    void set_enabled(bool enabled);

private:
    bool enabled_ = false;
    std::unordered_set<std::string> used_;
};

}
