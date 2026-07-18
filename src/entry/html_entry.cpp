#include "logger.hpp"
#include "utils/filesystem.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace guchho {

struct HtmlEntry {
    std::filesystem::path path;
    std::string content;
    std::vector<std::string> scripts;
    std::vector<std::string> styles;
    std::vector<std::string> assets;
};

HtmlEntry parse_html_entry(const std::filesystem::path &path) {
    HtmlEntry entry;
    entry.path = path;
    entry.content = read_file(path);

    log().info("Parsed HTML entry: " + path.string());
    return entry;
}

}
