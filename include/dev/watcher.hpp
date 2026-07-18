#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace guchho {

using FileChangeCallback = std::function<void(const std::filesystem::path &path)>;

class FileWatcher {
public:
    explicit FileWatcher(std::filesystem::path watch_dir);

    void start(FileChangeCallback callback);
    void stop();
    bool is_watching() const;
    void add_watch(const std::filesystem::path &path);
    void remove_watch(const std::filesystem::path &path);

private:
    std::filesystem::path watch_dir_;
    bool watching_ = false;
    std::vector<std::filesystem::path> watched_paths_;
};

}
