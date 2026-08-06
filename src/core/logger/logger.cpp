#include "guchho/logger.hpp"
#include "guchho/helpers.hpp"

#include <stdexcept>   // std::logic_error
#include <algorithm>
namespace guchho::logger{

std::string MsgKindToString(MsgKind kind) {
    switch (kind) {
        case MsgKind::kError:   return "ERROR";
        case MsgKind::kWarning: return "WARNING";
        case MsgKind::kInfo:    return "INFO";
        case MsgKind::kNote:    return "NOTE";
        case MsgKind::kDebug:   return "DEBUG";
        case MsgKind::kVerbose: return "VERBOSE";
    }
    return "";
}

std::string_view MsgKindToIcon(MsgKind kind)
{
    // Windows Command Prompt-এ কিছু Unicode character সমর্থিত নয়।
    if (IsProbablyWindowsCommandPrompt()) {
        switch (kind) {
            case MsgKind::kError:
                return "X";
            case MsgKind::kWarning:
                return "▲";
            case MsgKind::kInfo:
                return "►";
            case MsgKind::kNote:
                return "→";
            case MsgKind::kDebug:
                return "●";
            case MsgKind::kVerbose:
                return "♦";
        }

        throw std::logic_error("Internal error");
    }

    switch (kind) {
        case MsgKind::kError:
            return "✘";
        case MsgKind::kWarning:
            return "▲";
        case MsgKind::kInfo:
            return "▶";
        case MsgKind::kNote:
            return "→";
        case MsgKind::kDebug:
            return "●";
        case MsgKind::kVerbose:
            return "⬥";
    }

    throw std::logic_error("Internal error");
}

static uint32_t ReadUInt32LE(std::string_view bytes)
{
    return static_cast<uint32_t>(static_cast<unsigned char>(bytes[0])) |
           (static_cast<uint32_t>(static_cast<unsigned char>(bytes[1])) << 8) |
           (static_cast<uint32_t>(static_cast<unsigned char>(bytes[2])) << 16) |
           (static_cast<uint32_t>(static_cast<unsigned char>(bytes[3])) << 24);
}

// Packed import attribute data-কে decode করে
// key/value pair-এর array হিসেবে ফেরত দেয়.
//
// Data format: [key_length][key][value_length][value]
// যেখানে length 4-byte little-endian integer.
std::vector<ImportAttribute> ImportAttributes::DecodeIntoArray() const
{
    std::vector<ImportAttribute> result;

    if (packed_data.empty()) {
        return result;
    }

    std::string_view bytes = packed_data;

    while (!bytes.empty()) {
        uint32_t key_len = ReadUInt32LE(bytes.substr(0, 4));
        std::string key(bytes.substr(4, key_len));
        bytes.remove_prefix(4 + key_len);

        uint32_t value_len = ReadUInt32LE(bytes.substr(0, 4));
        std::string value(bytes.substr(4, value_len));
        bytes.remove_prefix(4 + value_len);

        result.push_back({std::move(key), std::move(value)});
    }

    return result;
}

// Packed import attribute data-কে decode করে
// key/value map হিসেবে ফেরত দেয়।
std::unordered_map<std::string, std::string> ImportAttributes::DecodeIntoMap() const
{
    auto arr = DecodeIntoArray();

    if (arr.empty()) {
        return {};
    }

    std::unordered_map<std::string, std::string> result;
    result.reserve(arr.size());

    for (auto& attr : arr) {
        result[attr.key] = std::move(attr.value);
    }

    return result;
}


ImportAttributes EncodeImportAttributes(const std::unordered_map<std::string, std::string>& value) {
    if (value.empty()) return {};
    std::vector<std::string> keys;
    keys.reserve(value.size());
    for (const auto& kv : value) keys.push_back(kv.first);
    std::sort(keys.begin(), keys.end());
    std::string sb;
    for (const auto& k : keys) {
        const auto& v = value.at(k);
        uint32_t kn = static_cast<uint32_t>(k.size());
        uint32_t vn = static_cast<uint32_t>(v.size());
        sb.append(reinterpret_cast<const char*>(&kn), 4);
        sb.append(k);
        sb.append(reinterpret_cast<const char*>(&vn), 4);
        sb.append(v);
    }
    return {std::move(sb)};
}

bool Path::IsDisabled() const {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(PathFlags::kPathDisabled)) != 0;
}

std::string Source::TextForRange(Range r) const {
    return contents.substr(r.loc.start, r.len);
}


Loc Source::LocBeforeWhitespace(Loc loc) const
{
    while (loc.start > 0) {
        auto [ch, width] =
            helpers::DecodeLastRuneInString(contents.substr(0, loc.start));

        if (ch != U' ' &&
            ch != U'\t' &&
            ch != U'\r' &&
            ch != U'\n') {
            break;
        }

        loc.start -= width;
    }

    return loc;
}

Range Source::RangeOfOperatorBefore(Loc loc, const std::string& op) const {
    auto text = contents.substr(0, static_cast<size_t>(loc.start));
    auto pos = text.rfind(op);
    if (pos != std::string::npos) {
        return {Loc{static_cast<int32_t>(pos)}, static_cast<int32_t>(op.size())};
    }
    return {loc, 0};
}

Range Source::RangeOfOperatorAfter(Loc loc, const std::string& op) const {
    auto text = contents.substr(static_cast<size_t>(loc.start));
    auto pos = text.find(op);
    if (pos != std::string::npos) {
        return {Loc{static_cast<int32_t>(loc.start + static_cast<int32_t>(pos))}, static_cast<int32_t>(op.size())};
    }
    return {loc, 0};
}

Range Source::RangeOfString(Loc loc) const {
    auto text = contents.substr(static_cast<size_t>(loc.start));
    if (text.empty()) return {loc, 0};
    char quote = text[0];
    if (quote == '"' || quote == '\'') {
        for (size_t i = 1; i < text.size(); i++) {
            char c = text[i];
            if (c == quote) {
                return {loc, static_cast<int32_t>(i + 1)};
            } else if (c == '\\') {
                i++;
            }
        }
    }
    if (quote == '`') {
        for (size_t i = 1; i < text.size(); i++) {
            char c = text[i];
            if (c == quote) {
                return {loc, static_cast<int32_t>(i + 1)};
            } else if (c == '\\') {
                i++;
            } else if (c == '$' && i + 1 < text.size() && text[i + 1] == '{') {
                break;
            }
        }
    }
    return {loc, 0};
}

Range Source::RangeOfNumber(Loc loc) const {
    auto text = contents.substr(static_cast<size_t>(loc.start));
    Range r{loc, 0};
    if (!text.empty()) {
        char c = text[0];
        if (c >= '0' && c <= '9') {
            r.len = 1;
            while (static_cast<size_t>(r.len) < text.size()) {
                c = text[static_cast<size_t>(r.len)];
                if ((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != '.' && c != '_') break;
                r.len++;
            }
        }
    }
    return r;
}

Range Source::RangeOfLegacyOctalEscape(Loc loc) const {
    auto text = contents.substr(static_cast<size_t>(loc.start));
    Range r{loc, 0};
    if (text.size() >= 2 && text[0] == '\\') {
        r.len = 2;
        while (r.len < 4 && static_cast<size_t>(r.len) < text.size()) {
            char c = text[static_cast<size_t>(r.len)];
            if (c < '0' || c > '9') break;
            r.len++;
        }
    }
    return r;
}



}

