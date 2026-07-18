#include "renamer/renamer.hpp"

namespace guchho::renamer {

void Renamer::add_reserved(const std::string &name) {
    reserved_.insert(name);
}

std::string Renamer::generate_name() {
    std::string name;
    size_t n = counter_++;
    do {
        name.insert(name.begin(), 'a' + (n % 26));
        n /= 26;
    } while (n > 0);
    return name;
}

std::string Renamer::rename(const std::string &original) {
    auto it = renames_.find(original);
    if (it != renames_.end()) return it->second;

    std::string new_name;
    do {
        new_name = generate_name();
    } while (reserved_.count(new_name));

    renames_[original] = new_name;
    return new_name;
}

std::unordered_map<std::string, std::string> Renamer::compute_renames(const std::unordered_set<std::string> &symbols) {
    std::unordered_map<std::string, std::string> result;
    for (const auto &sym : symbols) {
        if (reserved_.count(sym)) continue;
        std::string name;
        do {
            name = generate_name();
        } while (reserved_.count(name));
        result[sym] = name;
    }
    return result;
}

std::unordered_set<std::string> compute_keywords() {
    return {
        "break", "case", "catch", "class", "const", "continue",
        "debugger", "default", "delete", "do", "else", "enum",
        "export", "extends", "false", "finally", "for", "function",
        "if", "import", "in", "instanceof", "let", "new", "null",
        "return", "super", "switch", "this", "throw", "true", "try",
        "typeof", "var", "void", "while", "with", "yield",
    };
}

}
