#pragma once

#include <string>
#include <vector>

namespace guchho {

struct EmitOptions {
    std::string format = "esm";
    bool minify = false;
    bool source_map = false;
};

std::string emit_module(const std::string &code, const EmitOptions &options);
std::string emit_cjs(const std::string &code);
std::string emit_iife(const std::string &code, const std::string &global_name);

}
