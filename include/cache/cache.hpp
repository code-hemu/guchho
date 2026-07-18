#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

namespace guchho::cache {

class FSCache {
public:
    std::string read_file(const std::string &path, std::string &contents);

private:
    struct Entry {
        std::string contents;
        uint64_t mod_key;
        bool mod_key_valid = false;
    };

    std::unordered_map<std::string, Entry> entries_;
    std::mutex mutex_;
};

class JSCache {
public:
    // Placeholder for JS AST cache
};

class CSSCache {
public:
    // Placeholder for CSS AST cache
};

class CacheSet {
public:
    FSCache fs_cache;
    JSCache js_cache;
    CSSCache css_cache;
};

}
