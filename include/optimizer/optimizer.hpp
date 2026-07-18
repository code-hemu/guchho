#pragma once

#include <string>

namespace guchho {

struct OptimizeOptions {
    bool minify = false;
    bool tree_shake = false;
};

std::string optimize(const std::string &source, const OptimizeOptions &options);

}
