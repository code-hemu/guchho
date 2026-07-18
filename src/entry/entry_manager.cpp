#include "config/config.hpp"
#include "logger.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace guchho {

class EntryManager {
public:
    explicit EntryManager(const Config &config) : config_(config) {}

    std::vector<std::filesystem::path> resolve_entries() {
        std::vector<std::filesystem::path> entries;

        for (const auto &entry : config_.entry_points) {
            std::filesystem::path p(entry);

            if (!std::filesystem::exists(p)) {
                log().warn("Entry not found: " + entry);
                continue;
            }

            if (std::filesystem::is_regular_file(p)) {
                entries.push_back(p);
            } else if (std::filesystem::is_directory(p)) {
                for (const auto &file : std::filesystem::directory_iterator(p)) {
                    if (file.is_regular_file()) {
                        auto ext = file.path().extension();
                        if (ext == ".html" || ext == ".js" || ext == ".ts" || ext == ".jsx" || ext == ".tsx") {
                            entries.push_back(file.path());
                        }
                    }
                }
            }
        }

        return entries;
    }

private:
    const Config &config_;
};

}
