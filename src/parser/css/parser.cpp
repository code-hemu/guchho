#include "parser/css/parser.hpp"
#include "logger.hpp"
#include <string>
#include <vector>
#include <regex>

namespace guchho {

CssStylesheet parse_css(const std::string &source) {
    CssStylesheet sheet;
    sheet.source = source;

    std::regex import_re(R"(@import\s+["']([^"']+)["'])");
    std::smatch match;
    std::string::const_iterator it = source.cbegin();
    while (std::regex_search(it, source.cend(), match, import_re)) {
        sheet.imports.push_back(match[1]);
        it = match.suffix().first;
    }

    std::regex url_re(R"re(url\(["']?([^)"']+)["']?\))re");
    it = source.cbegin();
    while (std::regex_search(it, source.cend(), match, url_re)) {
        sheet.urls.push_back(match[1]);
        it = match.suffix().first;
    }

    log().info("Parsed CSS (" + std::to_string(sheet.imports.size()) + " imports, "
               + std::to_string(sheet.urls.size()) + " urls)");
    return sheet;
}

}
