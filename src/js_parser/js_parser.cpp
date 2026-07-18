#include "js_parser/js_parser.hpp"
#include "logger/logger.hpp"
#include <string>
#include <vector>
#include <regex>

namespace guchho::js {

JsModule parse_js(const std::string &source) {
    JsModule mod;
    mod.source = source;

    std::regex import_re(R"(import\s+(?:[\s\S]*?\s+from\s+)?["']([^"']+)["'])");
    std::smatch match;
    std::string::const_iterator it = source.cbegin();
    while (std::regex_search(it, source.cend(), match, import_re)) {
        mod.imports.push_back(match[1]);
        it = match.suffix().first;
    }

    std::regex export_re(R"(\bexport\s+)");
    it = source.cbegin();
    while (std::regex_search(it, source.cend(), match, export_re)) {
        mod.exports.push_back(match[0]);
        it = match.suffix().first;
    }

    log().info("Parsed JS module (" + std::to_string(mod.imports.size()) + " imports, "
               + std::to_string(mod.exports.size()) + " exports)");
    return mod;
}

}
