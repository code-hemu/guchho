#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace guchho {

struct Bundle {
    std::string code;
    std::string map;
    std::filesystem::path output_path;
};

class BundleWriter {
public:
    explicit BundleWriter(std::filesystem::path out_dir);
    bool write(const Bundle &bundle);
    bool write_all(const std::vector<Bundle> &bundles);

private:
    std::filesystem::path out_dir_;
};

}
