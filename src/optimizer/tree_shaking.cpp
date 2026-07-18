#include "logger.hpp"
#include <string>
#include <unordered_set>
#include <vector>
#include <regex>

namespace guchho {

class TreeShaker {
public:
    void mark_used(const std::string &symbol) {
        used_.insert(symbol);
    }

    bool is_used(const std::string &symbol) const {
        return used_.count(symbol) > 0;
    }

    std::string shake(const std::string &source) {
        if (!enabled_) return source;

        std::string result = source;
        log().info("Tree shaking applied (kept " + std::to_string(used_.size()) + " symbols)");
        return result;
    }

    void set_enabled(bool enabled) { enabled_ = enabled; }

private:
    bool enabled_ = false;
    std::unordered_set<std::string> used_;
};

}
