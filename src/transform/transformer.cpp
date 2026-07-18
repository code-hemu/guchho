#include "transform/transformer.hpp"
#include <string>

namespace guchho {

void Transformer::add_pass(TransformFn pass) {
    passes_.push_back(std::move(pass));
}

std::string Transformer::run(const std::string &source) const {
    std::string result = source;
    for (const auto &pass : passes_) {
        result = pass(result);
    }
    return result;
}

std::string strip_comments(const std::string &source) {
    std::string result;
    result.reserve(source.size());

    for (size_t i = 0; i < source.size(); ++i) {
        if (source[i] == '/' && i + 1 < source.size()) {
            if (source[i + 1] == '/') {
                i += 2;
                while (i < source.size() && source[i] != '\n') ++i;
                result += '\n';
                continue;
            }
            if (source[i + 1] == '*') {
                i += 2;
                while (i + 1 < source.size() && !(source[i] == '*' && source[i + 1] == '/')) ++i;
                i += 1;
                continue;
            }
        }
        result += source[i];
    }

    return result;
}

}
