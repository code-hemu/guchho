#include "config/config.hpp"
#include "logger/logger.hpp"
#include <string>

namespace guchho {

bool Config::validate() const {
    if (entry_points.empty()) {
        log().error("No entry points specified.");
        return false;
    }

    for (const auto &entry : entry_points) {
        if (entry.empty()) {
            log().error("Entry point path is empty.");
            return false;
        }
    }

    if (out_dir.empty()) {
        log().error("Output directory is empty.");
        return false;
    }

    if (out_file.empty()) {
        log().error("Output file name is empty.");
        return false;
    }

    if (format != "esm" && format != "cjs" && format != "iife") {
        log().error("Invalid format: " + format + ". Use esm, cjs, or iife.");
        return false;
    }

    return true;
}

}
