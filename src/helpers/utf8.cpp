#include "guchho/helpers.hpp"
#include <cstdint>

namespace guchho::helpers {

namespace {
constexpr int      kUTF8Max             = 4;
constexpr char32_t kRuneError           = 0xFFFD;
constexpr char32_t kMaxRune             = 0x10FFFF;
constexpr char32_t kMaxBMPCodePoint     = 0xFFFF;

constexpr char32_t kSurrogateHighStart  = 0xD800;
constexpr char32_t kSurrogateHighEnd    = 0xDBFF;
constexpr char32_t kSurrogateLowStart   = 0xDC00;
constexpr char32_t kSurrogateLowEnd     = 0xDFFF;
constexpr char32_t kSurrogateOffset     = 0x10000;
}

std::pair<char32_t, int> DecodeWTF8Rune(std::string_view text)
{
    const size_t n = text.size();

    if (n == 0) {
        return {kRuneError, 0};
    }

    const auto s0 = static_cast<unsigned char>(text[0]);

    // ASCII
    if (s0 < 0x80) {
        return {static_cast<char32_t>(s0), 1};
    }

    int width = 0;

    if ((s0 & 0xE0) == 0xC0) {
        width = 2;
    } else if ((s0 & 0xF0) == 0xE0) {
        width = 3;
    } else if ((s0 & 0xF8) == 0xF0) {
        width = 4;
    } else {
        return {kRuneError, 1};
    }

    if (n < static_cast<size_t>(width)) {
        return {kRuneError, 0};
    }

    const auto s1 = static_cast<unsigned char>(text[1]);

    if ((s1 & 0xC0) != 0x80) {
        return {kRuneError, 1};
    }

    if (width == 2) {
        const char32_t cp =
            (static_cast<char32_t>(s0 & 0x1F) << 6) |
            static_cast<char32_t>(s1 & 0x3F);

        // Overlong encoding
        if (cp < 0x80) {
            return {kRuneError, 1};
        }

        return {cp, 2};
    }

    const auto s2 = static_cast<unsigned char>(text[2]);

    if ((s2 & 0xC0) != 0x80) {
        return {kRuneError, 1};
    }

    if (width == 3) {
        const char32_t cp =
            (static_cast<char32_t>(s0 & 0x0F) << 12) |
            (static_cast<char32_t>(s1 & 0x3F) << 6) |
            static_cast<char32_t>(s2 & 0x3F);

        // Overlong encoding
        if (cp < 0x800) {
            return {kRuneError, 1};
        }

        // WTF-8 allows surrogate code points.
        return {cp, 3};
    }

    const auto s3 = static_cast<unsigned char>(text[3]);

    if ((s3 & 0xC0) != 0x80) {
        return {kRuneError, 1};
    }

    const char32_t cp =
        (static_cast<char32_t>(s0 & 0x07) << 18) |
        (static_cast<char32_t>(s1 & 0x3F) << 12) |
        (static_cast<char32_t>(s2 & 0x3F) << 6) |
        static_cast<char32_t>(s3 & 0x3F);

    // Overlong encoding
    if (cp < 0x10000) {
        return {kRuneError, 1};
    }

    // Outside the Unicode range.
    if (cp > kMaxRune) {
        return {kRuneError, 1};
    }

    return {cp, 4};
}

int EncodeWTF8Rune(char* buffer, char32_t code_point) 
{
    // Negative values are erroneous. Making it unsigned addresses the problem.
    uint32_t value = static_cast<uint32_t>(code_point);

    if (value <= 0x7F) {
        buffer[0] = static_cast<char>(code_point);
        return 1;
    }

    if (value <= 0x7FF) {
        buffer[0] = static_cast<char>(0xC0 | (value >> 6));
        buffer[1] = static_cast<char>(0x80 | (value & 0x3F));
        return 2;
    }

    if (value > kMaxRune) {
        code_point = kRuneError;
        value = static_cast<uint32_t>(code_point);
    }

    if (value <= 0xFFFF) {
        buffer[0] = static_cast<char>(0xE0 | (value >> 12));
        buffer[1] = static_cast<char>(0x80 | ((value >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (value & 0x3F));
        return 3;
    }

    buffer[0] = static_cast<char>(0xF0 | (value >> 18));
    buffer[1] = static_cast<char>(0x80 | ((value >> 12) & 0x3F));
    buffer[2] = static_cast<char>(0x80 | ((value >> 6) & 0x3F));
    buffer[3] = static_cast<char>(0x80 | (value & 0x3F));
    return 4;
}

bool ContainsNonBMPCodePoint(std::string_view text)
{
    size_t index = 0;

    while (index < text.size()) {
        auto [code_point, width] = DecodeWTF8Rune(text.substr(index));

        if (width == 0) {
            break;
        }

        if (code_point > kMaxBMPCodePoint) {
            return true;
        }

        index += static_cast<size_t>(width);
    }

    return false;
}

bool ContainsNonBMPCodePointUTF16(std::span<const char16_t> text)
{
    const size_t n = text.size();

    if (n > 0) {
        for (size_t i = 0; i + 1 < n; ++i) {

            char32_t c = static_cast<char32_t>(text[i]);
            // Check for a high surrogate
            if (c >= kSurrogateHighStart &&
                c <= kSurrogateHighEnd) {

                // Check for a low surrogate

                char32_t c2 = static_cast<char32_t>(text[i + 1]);
                
                if (c2 >= kSurrogateLowStart &&
                    c2 <= kSurrogateLowEnd) {
                    return true;
                }
            }
        }
    }

    return false;
}

std::u16string StringToUTF16(std::string_view text)
{
    std::u16string result;
    result.reserve(text.size());

    size_t i = 0;

    while (i < text.size()) {
        auto [cp, width] = DecodeWTF8Rune(text.substr(i));

        if (width == 0) {
            break;
        }

        if (cp <= 0xFFFF) {
            result.push_back(static_cast<char16_t>(cp));
        } else {
            cp -= kSurrogateOffset;

            result.push_back(static_cast<char16_t>(
                kSurrogateHighStart + ((cp >> 10) & 0x3FF)));

            result.push_back(static_cast<char16_t>(
                kSurrogateLowStart + (cp & 0x3FF)));
        }

        i += static_cast<size_t>(width);
    }

    return result;
}

std::string UTF16ToString(std::span<const char16_t> text)
{
    std::string result;
    result.reserve(text.size());

    const size_t n = text.size();

    for (size_t i = 0; i < n; ++i) {
        char32_t r1 = static_cast<char32_t>(text[i]);

        if (r1 >= kSurrogateHighStart &&
            r1 <= kSurrogateHighEnd &&
            i + 1 < n) {

            char32_t r2 = static_cast<char32_t>(text[i + 1]);

            if (r2 >= kSurrogateLowStart &&
                r2 <= kSurrogateLowEnd) {

                r1 = ((r1 - kSurrogateHighStart) << 10) |
                     (r2 - kSurrogateLowStart);

                r1 += kSurrogateOffset;
                ++i;
            }
        }

        char buffer[kUTF8Max];
        int width = EncodeWTF8Rune(buffer, r1);

        result.append(buffer, static_cast<size_t>(width));
    }

    return result;
}

std::tuple<std::string, char16_t, bool> UTF16ToStringWithValidation(std::span<const char16_t> text)
{
    std::string result;
    const size_t n = text.size();

    for (size_t i = 0; i < n; ++i) {
        char32_t r1 = static_cast<char32_t>(text[i]);

        if (r1 >= kSurrogateHighStart &&
            r1 <= kSurrogateHighEnd) {

            if (i + 1 < n) {
                char32_t r2 = static_cast<char32_t>(text[i + 1]);

                if (r2 >= kSurrogateLowStart &&
                    r2 <= kSurrogateLowEnd) {

                    r1 = ((r1 - kSurrogateHighStart) << 10) |
                         (r2 - kSurrogateLowStart);
                    r1 += kSurrogateOffset;

                    ++i;
                } else {
                    return {"", static_cast<char16_t>(r1), false};
                }
            } else {
                return {"", static_cast<char16_t>(r1), false};
            }
        } else if (r1 >= kSurrogateLowStart &&
                   r1 <= kSurrogateLowEnd) {
            return {"", static_cast<char16_t>(r1), false};
        }

        char buffer[kUTF8Max];
        const int width = EncodeWTF8Rune(buffer, r1);
        result.append(buffer, static_cast<size_t>(width));
    }

    return {result, 0, true};
}

bool UTF16EqualsString(std::span<const char16_t> text, std::string_view str)
{
    if (text.size() > str.size()) {
        // Strings can't be equal if UTF-16 encoding is longer than UTF-8 encoding.
        return false;
    }

    const size_t n = text.size();
    size_t j = 0;

    for (size_t i = 0; i < n; ++i) {
        char32_t r1 = static_cast<char32_t>(text[i]);

        if (r1 >= kSurrogateHighStart &&
            r1 <= kSurrogateHighEnd &&
            i + 1 < n) {

            char32_t r2 = static_cast<char32_t>(text[i + 1]);

            if (r2 >= kSurrogateLowStart &&
                r2 <= kSurrogateLowEnd) {

                r1 = ((r1 - kSurrogateHighStart) << 10) |
                     (r2 - kSurrogateLowStart);
                r1 += kSurrogateOffset;

                ++i;
            }
        }

        char buffer[kUTF8Max];
        const int width = EncodeWTF8Rune(buffer, r1);

        if (j + static_cast<size_t>(width) > str.size()) {
            return false;
        }

        for (int k = 0; k < width; ++k) {
            if (buffer[k] != str[j]) {
                return false;
            }

            ++j;
        }
    }

    return j == str.size();
}

bool UTF16EqualsUTF16(std::span<const char16_t> a, std::span<const char16_t> b)
{
    if (a.size() == b.size()) {
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }

        return true;
    }

    return false;
}

// UTF-8 string-এর শেষ rune decode করে।
// Return:
//   first  -> decoded Unicode code point
//   second -> rune-এর byte length
std::pair<char32_t, int> DecodeLastRuneInString(std::string_view text)
{
    if (text.empty()) {
        return {kRuneError, 0};
    }

    size_t i = text.size() - 1;

    // শেষ rune-এর প্রথম byte খুঁজে বের করো।
    while (i > 0 &&
           (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80) {
        --i;
    }

    auto [rune, width] = DecodeWTF8Rune(text.substr(i));

    // DecodeWTF8Rune() পুরো rune consume করেছে কিনা নিশ্চিত করো।
    if (width != static_cast<int>(text.size() - i)) {
        return {kRuneError, 1};
    }

    return {rune, width};
}

// UTF-8 string-এর প্রথম rune (Unicode code point) decode করে।
//
// Return:
//   first  -> decode করা Unicode code point
//   second -> rune decode করতে ব্যবহৃত byte-এর সংখ্যা
//
// যদি UTF-8 sequence অবৈধ (invalid) হয়:
//   {kRuneError, 1} return করে।
//
// যদি string খালি (empty) হয়:
//   {kRuneError, 0} return করে।
std::pair<char32_t, int> DecodeRuneInString(std::string_view text)
{
    if (text.empty()) {
        return {kRuneError, 0};
    }

    const unsigned char b0 = static_cast<unsigned char>(text[0]);

    // ASCII
    if (b0 < 0x80) {
        return {static_cast<char32_t>(b0), 1};
    }

    int width;

    if ((b0 & 0xE0) == 0xC0) {
        width = 2;
    } else if ((b0 & 0xF0) == 0xE0) {
        width = 3;
    } else if ((b0 & 0xF8) == 0xF0) {
        width = 4;
    } else {
        return {kRuneError, 1};
    }

    if (text.size() < static_cast<size_t>(width)) {
        return {kRuneError, 1};
    }

    char32_t rune = 0;

    switch (width) {
        case 2: {
            const unsigned char b1 = static_cast<unsigned char>(text[1]);

            if ((b1 & 0xC0) != 0x80) {
                return {kRuneError, 1};
            }

            rune =
                ((b0 & 0x1F) << 6) |
                (b1 & 0x3F);

            if (rune < 0x80) {
                return {kRuneError, 1};
            }

            break;
        }

        case 3: {
            const unsigned char b1 = static_cast<unsigned char>(text[1]);
            const unsigned char b2 = static_cast<unsigned char>(text[2]);

            if ((b1 & 0xC0) != 0x80 ||
                (b2 & 0xC0) != 0x80) {
                return {kRuneError, 1};
            }

            rune =
                ((b0 & 0x0F) << 12) |
                ((b1 & 0x3F) << 6) |
                (b2 & 0x3F);

            // Overlong বা surrogate invalid
            if (rune < 0x800 ||
                (rune >= 0xD800 && rune <= 0xDFFF)) {
                return {kRuneError, 1};
            }

            break;
        }

        case 4: {
            const unsigned char b1 = static_cast<unsigned char>(text[1]);
            const unsigned char b2 = static_cast<unsigned char>(text[2]);
            const unsigned char b3 = static_cast<unsigned char>(text[3]);

            if ((b1 & 0xC0) != 0x80 ||
                (b2 & 0xC0) != 0x80 ||
                (b3 & 0xC0) != 0x80) {
                return {kRuneError, 1};
            }

            rune =
                ((b0 & 0x07) << 18) |
                ((b1 & 0x3F) << 12) |
                ((b2 & 0x3F) << 6) |
                (b3 & 0x3F);

            if (rune < 0x10000 ||
                rune > 0x10FFFF) {
                return {kRuneError, 1};
            }

            break;
        }
    }

    return {rune, width};
}

} 
