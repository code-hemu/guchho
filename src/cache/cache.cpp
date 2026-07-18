#include "cache/cache.hpp"

namespace guchho::cache {

std::string FSCache::read_file(const std::string &path, std::string &contents) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = entries_.find(path);
    if (it != entries_.end()) {
        contents = it->second.contents;
        return {};
    }
    return "not found";
}

}
