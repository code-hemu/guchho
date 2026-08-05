#include "helpers/utf8.hpp"
#include <string>
#include <utility>
#include <cstdint>

namespace guchho::helpers {

namespace {

constexpr char32_t kRuneError = 0xFFFD;
constexpr char32_t kMaxRune = 0x10FFFF;

}


std::pair<char32_t, int> DecodeWTF8Rune(std::string_view s)
{
    size_t n = s.size();

    if (n == 0) {
        return {kRuneError, 0};
    }

    unsigned char s0 = static_cast<unsigned char>(s[0]);

    // ASCII (1 byte)
    if (s0 < 0x80) {
        return {static_cast<char32_t>(s0), 1};
    }

    int width = 0;

    // Determine UTF-8 sequence length
    if ((s0 & 0xE0) == 0xC0) {
        width = 2;
    } 
    else if ((s0 & 0xF0) == 0xE0) {
        width = 3;
    } 
    else if ((s0 & 0xF8) == 0xF0) {
        width = 4;
    } 
    else {
        return {kRuneError, 1};
    }

    if (n < static_cast<size_t>(width)) {
        return {kRuneError, 0};
    }

    unsigned char s1 = static_cast<unsigned char>(s[1]);

    if ((s1 & 0xC0) != 0x80) {
        return {kRuneError, 1};
    }


    // 2-byte UTF-8
    if (width == 2) {
        char32_t cp =
            (static_cast<char32_t>(s0 & 0x1F) << 6) |
            static_cast<char32_t>(s1 & 0x3F);

        // Overlong encoding
        if (cp < 0x80) {
            return {kRuneError, 1};
        }

        return {cp, 2};
    }


    unsigned char s2 = static_cast<unsigned char>(s[2]);

    if ((s2 & 0xC0) != 0x80) {
        return {kRuneError, 1};
    }


    // 3-byte UTF-8
    if (width == 3) {
        char32_t cp =
            (static_cast<char32_t>(s0 & 0x0F) << 12) |
            (static_cast<char32_t>(s1 & 0x3F) << 6) |
            static_cast<char32_t>(s2 & 0x3F);

        // Overlong encoding
        if (cp < 0x800) {
            return {kRuneError, 1};
        }

        return {cp, 3};
    }


    unsigned char s3 = static_cast<unsigned char>(s[3]);

    if ((s3 & 0xC0) != 0x80) {
        return {kRuneError, 1};
    }


    // 4-byte UTF-8
    char32_t cp =
        (static_cast<char32_t>(s0 & 0x07) << 18) |
        (static_cast<char32_t>(s1 & 0x3F) << 12) |
        (static_cast<char32_t>(s2 & 0x3F) << 6) |
        static_cast<char32_t>(s3 & 0x3F);


    // Invalid range
    if (cp < 0x10000 || cp > kMaxRune) {
        return {kRuneError, 1};
    }

    return {cp, 4};
}

bool ContainsNonBMPCodePoint(std::string_view text)
{
    size_t i = 0;
    size_t n = text.size();
    while (i < n) {
        auto [cp, width] = DecodeWTF8Rune(text.substr(i));
        if (width == 0) break;
        if (cp > 0xFFFF) return true;
        i += static_cast<size_t>(width);
    }
    return false;
}


}


