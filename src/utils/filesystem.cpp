#include "utils/filesystem.hpp"
#include "version.hpp"
#include <fstream>
#include <sstream>

namespace guchho {
    namespace fs = std::filesystem;

    std::string read_file(const fs::path &path) {
        std::ifstream file(path);
        if (!file.is_open()) return {};
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    bool write_file(const fs::path &path, const std::string &content) {
        fs::create_directories(path.parent_path());
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << content;
        return file.good();
    }

    bool file_exists(const fs::path &path) {
        return fs::exists(path) && fs::is_regular_file(path);
    }

    std::vector<fs::path> list_files(const fs::path &dir, const std::string &extension) {
        std::vector<fs::path> result;
        if (!fs::exists(dir) || !fs::is_directory(dir)) return result;
        for (const auto &entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                if (extension.empty() || entry.path().extension() == extension)
                    result.push_back(entry.path());
            }
        }
        return result;
    }
}
