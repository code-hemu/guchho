#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "cli/cli.hpp"
#include "version.hpp"

namespace guchho {

void print_usage() {
    std::cout << "Usage: guchho [command] [options]\n\n"
              << "Commands:\n"
              << "  build    Build the project\n"
              << "  watch    Build and watch for changes\n"
              << "  init     Create a default config\n"
              << "  version  Show version\n"
              << "  help     Show this help\n";
}

void print_version() {
    std::cout << "guchho v" << version_string() << std::endl;
}

int run_cli(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty() || args[0] == "help" || args[0] == "--help") {
        print_usage();
        return 0;
    }

    if (args[0] == "version") {
        print_version();
        return 0;
    }

    if (args[0] == "--json") {
        std::string input, line;
        while (std::getline(std::cin, line)) {
            input += line;
        }

        bool is_transform = input.find("transform") != std::string::npos;

        if (is_transform) {
            std::cout << R"({"code":"","errors":[],"warnings":[]})" << std::endl;
        } else {
            std::cout << R"({"errors":[],"warnings":[],"outputFiles":[]})" << std::endl;
        }
        return 0;
    }

    if (args[0] == "init") {
        std::cout << "Creating default guchho config..." << std::endl;
        return 0;
    }

    if (args[0] == "build") {
        std::cout << "Building project..." << std::endl;
        return 0;
    }

    if (args[0] == "watch") {
        std::cout << "Watching for changes..." << std::endl;
        return 0;
    }

    std::cerr << "Unknown command: " << args[0] << std::endl;
    print_usage();
    return 1;
}

}
