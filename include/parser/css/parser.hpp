#pragma once
#include <string>
#include <vector>

namespace guchho {

struct CssStylesheet {
    std::string source;
    std::vector<std::string> imports;
    std::vector<std::string> urls;
};

CssStylesheet parse_css(const std::string &source);

}
