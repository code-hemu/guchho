#include "logger/logger.hpp"
#include "fs/fs.hpp"
#include <string>
#include <filesystem>

namespace guchho {
    using namespace guchho::fs;

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
