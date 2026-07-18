#pragma once

#include <string>
#include <vector>

namespace guchho::js {

struct PrintOptions {
    std::string format = "esm";
    bool minify = false;
    bool source_map = false;
};

std::string print_js(const std::string &code, const PrintOptions &options = {});
std::string print_js_module(const std::string &code);
std::string print_js_cjs(const std::string &code);
std::string print_js_iife(const std::string &code, const std::string &global_name);

}
