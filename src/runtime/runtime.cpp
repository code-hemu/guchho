#include "runtime/runtime.hpp"

namespace guchho::runtime {

std::string get_runtime_source() {
    return R"(
function __pow(base, exp) {
    return Math.pow(base, exp);
}
function __rest(arr, idx) {
    return Array.prototype.slice.call(arr, idx);
}
function __spread(...args) {
    return Array.prototype.concat.apply([], args);
}
)";
}

}
