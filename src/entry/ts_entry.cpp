#include "logger.hpp"
#include "utils/filesystem.hpp"
#include <string>
#include <filesystem>

namespace guchho {

struct TsEntry {
    std::filesystem::path path;
    std::string content;
    std::string target = "esnext";
    bool jsx = false;
};

TsEntry parse_ts_entry(const std::filesystem::path &path) {
    TsEntry entry;
    entry.path = path;
    entry.content = read_file(path);

    auto ext = path.extension();
    entry.jsx = (ext == ".tsx");

    log().info("Parsed TS entry: " + path.string());
    return entry;
}

}
