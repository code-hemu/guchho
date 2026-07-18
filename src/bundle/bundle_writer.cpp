#include "bundle/bundle_writer.hpp"
#include "logger/logger.hpp"
#include "fs/fs.hpp"

namespace guchho {
    using namespace guchho::fs;

BundleWriter::BundleWriter(std::filesystem::path out_dir)
    : out_dir_(std::move(out_dir)) {}

bool BundleWriter::write(const Bundle &bundle) {
    auto path = out_dir_ / bundle.output_path;
    if (!write_file(path, bundle.code)) {
        log().error("Failed to write bundle: " + path.string());
        return false;
    }
    log().info("Written bundle: " + path.string());
    return true;
}

bool BundleWriter::write_all(const std::vector<Bundle> &bundles) {
    bool ok = true;
    for (const auto &bundle : bundles) {
        if (!write(bundle)) ok = false;
    }
    return ok;
}

}
