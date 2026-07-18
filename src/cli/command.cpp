#include <iostream>
#include <string>
#include <vector>
#include "cli/command.hpp"
#include "config/config.hpp"
#include "version.hpp"

namespace guchho {

int cmd_build(const std::vector<std::string> &args) {
    Config config;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--out-dir" && i + 1 < args.size()) {
            config.out_dir = args[++i];
        } else if (args[i] == "--out-file" && i + 1 < args.size()) {
            config.out_file = args[++i];
        } else if (args[i] == "--minify") {
            config.minify = true;
        } else if (args[i] == "--source-maps") {
            config.source_maps = true;
        } else if (args[i] == "--tree-shaking") {
            config.tree_shaking = true;
        } else if (args[i] == "--format" && i + 1 < args.size()) {
            config.format = args[++i];
        } else if (args[i][0] != '-') {
            config.entry_points.push_back(args[i]);
        }
    }

    if (!config.validate()) {
        std::cerr << "Invalid configuration." << std::endl;
        return 1;
    }

    std::cout << "Build complete." << std::endl;
    return 0;
}

int cmd_watch(const std::vector<std::string> &args) {
    std::cout << "Watch mode not yet implemented." << std::endl;
    return 0;
}

int cmd_init(const std::vector<std::string> &args) {
    std::cout << "Init not yet implemented." << std::endl;
    return 0;
}

}
