#include "config/config.hpp"
#include "logger/logger.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace guchho {

bool Config::load(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        log().error("Config file not found: " + path.string());
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        log().error("Failed to open config: " + path.string());
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    if (content.empty()) {
        log().warn("Config file is empty, using defaults.");
        return true;
    }

    log().info("Loaded config from " + path.string());
    return true;
}

}
