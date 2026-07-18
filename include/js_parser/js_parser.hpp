#pragma once
#include <string>
#include <vector>

namespace guchho::js {

struct JsModule {
    std::string source;
    std::vector<std::string> imports;
    std::vector<std::string> exports;
};

JsModule parse_js(const std::string &source);

}
