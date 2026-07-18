#pragma once

#include <string>

namespace guchho {

constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

inline std::string version_string() {
    return std::to_string(VERSION_MAJOR) + "."
         + std::to_string(VERSION_MINOR) + "."
         + std::to_string(VERSION_PATCH);
}

}
