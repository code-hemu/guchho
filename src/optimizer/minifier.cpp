#include "logger.hpp"
#include "transform/transformer.hpp"
#include <string>
#include <regex>

namespace guchho {

std::string minify_js(const std::string &source) {
    std::string result = strip_comments(source);

    result = std::regex_replace(result, std::regex("\\s+"), " ");
    result = std::regex_replace(result, std::regex("\\s*([{}();,:])\\s*"), "$1");
    result = std::regex_replace(result, std::regex("\\s+\\.\\s+"), ".");
    result = std::regex_replace(result, std::regex("\\s*\\[\\s*"), "[");
    result = std::regex_replace(result, std::regex("\\s*\\]\\s*"), "]");

    log().info("Minified: " + std::to_string(source.size()) + " -> " + std::to_string(result.size()) + " bytes");
    return result;
}

std::string minify_css(const std::string &source) {
    std::string result;

    result = std::regex_replace(source, std::regex("/\\*[\\s\\S]*?\\*/"), "");
    result = std::regex_replace(result, std::regex("\\s+"), " ");
    result = std::regex_replace(result, std::regex("\\s*([{}();,:])\\s*"), "$1");
    result = std::regex_replace(result, std::regex(";\\}"), "}");

    log().info("Minified CSS: " + std::to_string(source.size()) + " -> " + std::to_string(result.size()) + " bytes");
    return result;
}

}
