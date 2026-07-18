#include "dev/watcher.hpp"
#include "logger.hpp"
#include <algorithm>

namespace guchho {

FileWatcher::FileWatcher(std::filesystem::path watch_dir)
    : watch_dir_(std::move(watch_dir)) {}

void FileWatcher::start(FileChangeCallback callback) {
    watching_ = true;
    log().info("File watcher started on " + watch_dir_.string());
}

void FileWatcher::stop() {
    watching_ = false;
    log().info("File watcher stopped.");
}

bool FileWatcher::is_watching() const { return watching_; }

void FileWatcher::add_watch(const std::filesystem::path &path) {
    watched_paths_.push_back(path);
}

void FileWatcher::remove_watch(const std::filesystem::path &path) {
    auto it = std::find(watched_paths_.begin(), watched_paths_.end(), path);
    if (it != watched_paths_.end()) watched_paths_.erase(it);
}

}
