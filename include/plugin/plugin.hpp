#pragma once

#include <string>

namespace guchho {

struct Plugin {
    std::string name;

    virtual ~Plugin() = default;
    virtual std::string on_resolve(const std::string &specifier, const std::string &importer);
    virtual std::string on_load(const std::string &path);
    virtual std::string on_transform(const std::string &code, const std::string &path);
};

}
