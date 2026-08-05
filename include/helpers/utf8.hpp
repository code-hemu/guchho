#pragma once

#include <utility>
#include <tuple>
#include <string_view>
#include <string>
#include <span>

namespace guchho::helpers {
std::pair<char32_t, int>                DecodeWTF8Rune(std::string_view text);
int                                     EncodeWTF8Rune(char* buffer, char32_t code_point);
bool                                    ContainsNonBMPCodePoint(std::string_view text);
bool                                    ContainsNonBMPCodePointUTF16(std::span<const char16_t> text);
std::u16string                          StringToUTF16(std::string_view text);
std::string                             UTF16ToString(std::span<const char16_t> text);
std::tuple<std::string, char16_t, bool> UTF16ToStringWithValidation(std::span<const char16_t> text);
bool                                    UTF16EqualsString(std::span<const char16_t> text, std::string_view str);
bool                                    UTF16EqualsUTF16(std::span<const char16_t> a, std::span<const char16_t> b);
}
