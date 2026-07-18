#include "fs/fs.hpp"
#include <fstream>
#include <sstream>

namespace guchho::fs {
    namespace stdfs = std::filesystem;

    std::string read_file(const stdfs::path &path) {
        std::ifstream file(path);
        if (!file.is_open()) return {};
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    bool write_file(const stdfs::path &path, const std::string &content) {
        stdfs::create_directories(path.parent_path());
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << content;
        return file.good();
    }

    bool file_exists(const stdfs::path &path) {
        return stdfs::exists(path) && stdfs::is_regular_file(path);
    }

    std::vector<stdfs::path> list_files(const stdfs::path &dir, const std::string &extension) {
        std::vector<stdfs::path> result;
        if (!stdfs::exists(dir) || !stdfs::is_directory(dir)) return result;
        for (const auto &entry : stdfs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                if (extension.empty() || entry.path().extension() == extension)
                    result.push_back(entry.path());
            }
        }
        return result;
    }
}
