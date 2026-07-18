#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace guchho::fs {

std::string read_file(const std::filesystem::path &path);
bool write_file(const std::filesystem::path &path, const std::string &content);
bool file_exists(const std::filesystem::path &path);
std::vector<std::filesystem::path> list_files(const std::filesystem::path &dir, const std::string &extension = "");

}
