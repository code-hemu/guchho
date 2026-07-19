#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace guchho::js {

// Parse a global name expression like "globalThis.process.env.NODE_ENV"
// Returns the parts of the dotted name, or empty vector on failure.
std::vector<std::string> parseGlobalName(std::string_view source);

} // namespace guchho::js
