#pragma once

#include <string>

namespace guchho::css {

struct CssPrintOptions {
    bool minify = false;
    bool source_map = false;
};

std::string print_css(const std::string &code, const CssPrintOptions &options = {});

}
