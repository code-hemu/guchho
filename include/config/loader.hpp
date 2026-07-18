#pragma once

#include <filesystem>

namespace guchho {

struct Config;

Config load_config(const std::filesystem::path &path);

}
