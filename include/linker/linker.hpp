#pragma once

#include <string>
#include <vector>

namespace guchho::linker {

struct LinkOptions {
    bool minify = false;
    bool source_map = false;
    bool tree_shaking = false;
};

struct OutputFile {
    std::string path;
    std::string code;
    std::string map;
};

class Linker {
public:
    explicit Linker(LinkOptions options);
    std::vector<OutputFile> link(const std::vector<std::string> &modules);

private:
    LinkOptions options_;
};

}
