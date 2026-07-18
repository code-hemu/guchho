#pragma once

#include <functional>
#include <string>
#include <vector>

namespace guchho {

using TransformFn = std::function<std::string(const std::string &)>;

class Transformer {
public:
    void add_pass(TransformFn pass);
    std::string run(const std::string &source) const;

private:
    std::vector<TransformFn> passes_;
};

std::string strip_comments(const std::string &source);

}
