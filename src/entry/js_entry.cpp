#include "logger.hpp"
#include "utils/filesystem.hpp"
#include <string>
#include <filesystem>

namespace guchho {

struct JsEntry {
    std::filesystem::path path;
    std::string content;
    bool is_typescript = false;
    bool is_jsx = false;
};

JsEntry parse_js_entry(const std::filesystem::path &path) {
    JsEntry entry;
    entry.path = path;
    entry.content = read_file(path);

    auto ext = path.extension();
    entry.is_typescript = (ext == ".ts" || ext == ".tsx");
    entry.is_jsx = (ext == ".jsx" || ext == ".tsx");

    log().info("Parsed JS entry: " + path.string());
    return entry;
}

}
