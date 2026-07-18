#include "optimizer/optimizer.hpp"
#include "optimizer/minifier.hpp"
#include "optimizer/tree_shaking.hpp"

namespace guchho {

std::string optimize(const std::string &source, const OptimizeOptions &options) {
    std::string result = source;
    TreeShaker shaker;
    shaker.set_enabled(options.tree_shake);
    result = shaker.shake(result);
    if (options.minify) {
        result = minify_js(result);
    }
    return result;
}

}
