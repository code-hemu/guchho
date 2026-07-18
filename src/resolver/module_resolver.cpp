#include "resolver/module_resolver.hpp"
#include "logger/logger.hpp"
#include "fs/fs.hpp"

namespace guchho {
    using namespace guchho::fs;

ModuleResolver::ModuleResolver(std::vector<std::filesystem::path> lookup_paths)
    : lookup_paths_(std::move(lookup_paths)) {}

void ModuleResolver::add_alias(const std::string &name, const std::filesystem::path &path) {
    aliases_[name] = path;
}

std::filesystem::path ModuleResolver::resolve(const std::string &specifier,
                                               const std::filesystem::path &importer) {
    if (aliases_.count(specifier)) {
        return aliases_[specifier];
    }

    if (specifier.starts_with("./") || specifier.starts_with("../")) {
        auto resolved = importer.parent_path() / specifier;
        return resolve_path(resolved);
    }

    if (specifier.starts_with("/")) {
        return resolve_path(std::filesystem::path(specifier));
    }

    for (const auto &lookup : lookup_paths_) {
        auto pkg = lookup / "node_modules" / specifier;
        if (file_exists(pkg / "index.js")) {
            return pkg / "index.js";
        }
        if (file_exists(pkg / "index.mjs")) {
            return pkg / "index.mjs";
        }
        if (file_exists(pkg.string() + ".js")) {
            return std::filesystem::path(pkg.string() + ".js");
        }
    }

    log().warn("Module not found: " + specifier + " (imported by " + importer.string() + ")");
    return {};
}

std::filesystem::path ModuleResolver::resolve_path(std::filesystem::path base) {
    if (file_exists(base)) return base;
    if (file_exists(base.string() + ".js")) return base.string() + ".js";
    if (file_exists(base.string() + ".ts")) return base.string() + ".ts";
    if (file_exists(base.string() + ".json")) return base.string() + ".json";
    if (file_exists(base / "index.js")) return base / "index.js";
    if (file_exists(base / "index.ts")) return base / "index.ts";
    return base;
}

}
