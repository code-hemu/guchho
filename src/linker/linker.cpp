#include "linker/linker.hpp"
#include "logger/logger.hpp"

namespace guchho::linker {

Linker::Linker(LinkOptions options)
    : options_(std::move(options)) {}

std::vector<OutputFile> Linker::link(const std::vector<std::string> &modules) {
    std::vector<OutputFile> outputs;

    for (size_t i = 0; i < modules.size(); ++i) {
        OutputFile file;
        file.path = "output_" + std::to_string(i) + ".js";
        file.code = modules[i];
        outputs.push_back(std::move(file));
    }

    log().info("Linked " + std::to_string(outputs.size()) + " output files");
    return outputs;
}

}
