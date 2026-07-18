#include "bundle/emitter.hpp"

namespace guchho {

std::string emit_module(const std::string &code, const EmitOptions &options) {
    return code;
}

std::string emit_cjs(const std::string &code) {
    return code;
}

std::string emit_iife(const std::string &code, const std::string &global_name) {
    return code;
}

}
