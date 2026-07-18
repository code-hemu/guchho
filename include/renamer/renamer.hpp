#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace guchho::renamer {

class Renamer {
public:
    void add_reserved(const std::string &name);
    std::string rename(const std::string &original);
    std::unordered_map<std::string, std::string> compute_renames(const std::unordered_set<std::string> &symbols);

private:
    std::unordered_set<std::string> reserved_;
    std::unordered_map<std::string, std::string> renames_;
    size_t counter_ = 0;

    std::string generate_name();
};

std::unordered_set<std::string> compute_keywords();

}
