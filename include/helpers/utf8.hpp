#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace guchho::helpers {

std::pair<char32_t, int> DecodeWTF8Rune(std::string_view s);

bool ContainsNonBMPCodePoint(std::string_view text);



}