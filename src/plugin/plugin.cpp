#include "plugin/plugin.hpp"

namespace guchho {

std::string Plugin::on_resolve(const std::string &specifier, const std::string &importer) {
    return specifier;
}

std::string Plugin::on_load(const std::string &path) {
    return {};
}

std::string Plugin::on_transform(const std::string &code, const std::string &path) {
    return code;
}

}
