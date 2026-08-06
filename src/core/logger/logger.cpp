#include "guchho/logger.hpp"
#include "guchho/helpers.hpp"

#include <format>
#include <sstream>
#include <cstdlib>
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

std::string Source::CommentTextWithoutIndent(const Range& r) const
{
    std::string text = contents.substr(r.loc.start, r.End() - r.loc.start);

    if (text.size() < 2 || !text.starts_with("/*")) {
        return text;
    }

    std::string_view prefix(contents.data(), r.loc.start);

    // Figure out the initial indent.
    int indent = 0;

    while (!prefix.empty()) {
        auto [c, width] = helpers::DecodeLastRuneInString(prefix);

        if (c == U'\r' ||
            c == U'\n' ||
            c == U'\u2028' ||
            c == U'\u2029') {
            break;
        }

        prefix.remove_suffix(width);
        ++indent;
    }

    // Split the comment into lines.
    std::vector<std::string> lines;
    size_t start = 0;
    size_t i = 0;

    while (i < text.size()) {
        auto [c, width] = helpers::DecodeWTF8Rune(
            std::string_view(text).substr(i));

        switch (c) {
            case U'\r':
            case U'\n':
                // Don't double-append for Windows style "\r\n".
                if (start <= i) {
                    lines.emplace_back(text.substr(start, i - start));
                }

                start = i + width;

                // Ignore the second part of "\r\n".
                if (c == U'\r' &&
                    start < text.size() &&
                    text[start] == '\n') {
                    ++start;
                    ++i;
                }
                break;

            case U'\u2028':
            case U'\u2029':
                lines.emplace_back(text.substr(start, i - start));
                start = i + width;
                break;
        }

        i += width;
    }

    lines.emplace_back(text.substr(start));

    // Find the minimum indent over all lines after the first line.
    for (size_t lineIndex = 1; lineIndex < lines.size(); ++lineIndex) {
        int lineIndent = 0;

        for (char ch : lines[lineIndex]) {
            if (ch != ' ' && ch != '\t') {
                break;
            }

            ++lineIndent;
        }

        indent = std::min(indent, lineIndent);
    }

    // Trim the indent off of all lines after the first line.
    for (size_t lineIndex = 1; lineIndex < lines.size(); ++lineIndex) {
        lines[lineIndex].erase(0, std::min<size_t>(indent, lines[lineIndex].size()));
    }

    // Join lines with '\n'.
    std::string result;

    for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        if (lineIndex > 0) {
            result.push_back('\n');
        }

        result += lines[lineIndex];
    }

    return result;
}

int32_t Range::End() const {
    return loc.start + len;
}

void Range::ExpandBy(const Range& b) {
    if (len == 0) {
        *this = b;
    } else {
        int32_t end = End();
        int32_t nEnd = b.End();
        if (nEnd > end) end = nEnd;
        if (b.loc.start < loc.start) loc.start = b.loc.start;
        len = end - loc.start;
    }
}

// একটি path-কে তিনটি অংশে ভাগ করে:
//   - dir  : directory path
//   - base : extension ছাড়া filename
//   - ext  : file extension
//
// Functionটি Unix ('/') এবং Windows ('\\') উভয় ধরনের
// path separator সমর্থন করে। Absolute path-এর root
// ঠিক রাখে এবং filename-এর extension আলাদা করে বের করে।
//
// ".module.css" ফাইলের ক্ষেত্রে ".css" নয়,
// পুরো ".module.css" কে একটি একক extension হিসেবে গণনা করে।
void PlatformIndependentPathDirBaseExt(const std::string& path, std::string& dir, std::string& base, std::string& ext) {
    int absRootSlash = -1;

    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
        absRootSlash = 0;
    } else if (path.size() > 2 && path[1] == ':' && (path[2] == '/' || path[2] == '\\')) {
        char c = path[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            absRootSlash = 2;
        }
    }

    std::string p = path;
    while (true) {
        auto i = p.rfind('/');
        auto j = p.rfind('\\');
        size_t slash = (i == std::string::npos) ? j : (j == std::string::npos) ? i : std::max(i, j);

        if (slash == std::string::npos) {
            base = p;
            break;
        }

        int s = static_cast<int>(slash);
        if (s == absRootSlash) {
            dir = p.substr(0, static_cast<size_t>(s + 1));
            base = p.substr(static_cast<size_t>(s + 1));
            break;
        }
        if (static_cast<size_t>(s + 1) != p.size()) {
            dir = p.substr(0, static_cast<size_t>(s));
            base = p.substr(static_cast<size_t>(s + 1));
            break;
        }
        p = p.substr(0, static_cast<size_t>(s));
    }

    auto dot = base.rfind('.');
    if (dot != std::string::npos) {
        ext = base.substr(dot);
        if (ext == ".css") {
            auto dot2 = base.rfind('.', dot - 1);
            if (dot2 != std::string::npos && base.substr(dot2) == ".module.css") {
                dot = dot2;
                ext = base.substr(dot);
            }
        }
        base = base.substr(0, dot);
    }
}

// নির্বাচিত style-এর path ফেরত দেয়।
// kRelPath হলে relative path (যদি খালি না হয়), অন্যথায় absolute path।
std::string PrettyPaths::Select(PathStyle style) const {
    if (style == PathStyle::kRelPath && !rel.empty()) {
        return rel;
    }
    return abs;
}

// Message ID-এর জন্য কোনো log level override প্রয়োগ করে।
//
// যদি overrides-এ দেওয়া ID পাওয়া যায়, তাহলে সংশ্লিষ্ট
// LogLevel-কে উপযুক্ত MsgKind-এ রূপান্তর করে ফেরত দেয়।
// Override না থাকলে মূল MsgKind-ই ফেরত দেয়।
// Silent level-এর ক্ষেত্রে message suppress করার জন্য
// একটি বিশেষ মান ফেরত দেওয়া হয়।
MsgKind AllowOverride(const std::unordered_map<MsgID, LogLevel>& overrides, MsgID id, MsgKind kind) {
    auto it = overrides.find(id);
    if (it != overrides.end()) {
        switch (it->second) {
            case LogLevel::kVerbose: return MsgKind::kVerbose;
            case LogLevel::kDebug:   return MsgKind::kDebug;
            case LogLevel::kInfo:    return MsgKind::kInfo;
            case LogLevel::kWarning: return MsgKind::kWarning;
            case LogLevel::kError:   return MsgKind::kError;
            default:
                return static_cast<MsgKind>(0); // silent
        }
    }
    return kind;
}

// Command-line arguments পড়ে OutputOptions তৈরি করে।
//
// সমর্থিত option (যেমন --color, --log-level) শনাক্ত করে
// সংশ্লিষ্ট output settings প্রয়োগ করে। উল্লেখ না করা
// option-গুলোর জন্য default মান ব্যবহার করা হয়।
OutputOptions OutputOptionsForArgs(const std::vector<std::string>& os_args) {
    OutputOptions options;
    options.include_source = true;

    for (const auto& arg : os_args) {
        if (arg == "--color=false") {
            options.color = UseColor::kColorNever;
        } else if (arg == "--color=true" || arg == "--color") {
            options.color = UseColor::kColorAlways;
        } else if (arg == "--log-level=info") {
            options.log_level = LogLevel::kInfo;
        } else if (arg == "--log-level=warning") {
            options.log_level = LogLevel::kWarning;
        } else if (arg == "--log-level=error") {
            options.log_level = LogLevel::kError;
        } else if (arg == "--log-level=silent") {
            options.log_level = LogLevel::kSilent;
        }
    }

    return options;
}


void Log::AddError(LineColumnTracker* tracker, Range r, const std::string& text) {
    Msg msg;
    msg.kind = MsgKind::kError;
    if (tracker) {
        msg.data = tracker->MakeMsgData(r, text);
    } else {
        msg.data.text = text;
    }
    add_msg(std::move(msg));
}

void Log::AddID(MsgID id, MsgKind kind, LineColumnTracker* tracker, Range r, const std::string& text) {
    MsgKind overrideKind = AllowOverride(overrides, id, kind);
    if (static_cast<uint8_t>(overrideKind) == 0 && overrides.count(id)) return;
    Msg msg;
    msg.id = id;
    msg.kind = overrideKind;
    if (tracker) {
        msg.data = tracker->MakeMsgData(r, text);
    } else {
        msg.data.text = text;
    }
    add_msg(std::move(msg));
}

void Log::AddErrorWithNotes(LineColumnTracker* tracker, Range r, const std::string& text, const std::vector<MsgData>& notes) {
    Msg msg;
    msg.kind = MsgKind::kError;
    msg.notes = notes;
    if (tracker) {
        msg.data = tracker->MakeMsgData(r, text);
    } else {
        msg.data.text = text;
    }
    add_msg(std::move(msg));
}

void Log::AddIDWithNotes(MsgID id, MsgKind kind, LineColumnTracker* tracker, Range r, const std::string& text, const std::vector<MsgData>& notes) {
    MsgKind overrideKind = AllowOverride(overrides, id, kind);
    if (static_cast<uint8_t>(overrideKind) == 0 && overrides.count(id)) return;
    Msg msg;
    msg.id = id;
    msg.kind = overrideKind;
    msg.notes = notes;
    if (tracker) {
        msg.data = tracker->MakeMsgData(r, text);
    } else {
        msg.data.text = text;
    }
    add_msg(std::move(msg));
}

void Log::AddMsgID(MsgID id, const Msg& msg) {
    MsgKind overrideKind = AllowOverride(overrides, id, msg.kind);
    if (static_cast<uint8_t>(overrideKind) == 0 && overrides.count(id)) return;
    Msg m = msg;
    m.id = id;
    m.kind = overrideKind;
    add_msg(std::move(m));
}

std::string Plural(const std::string& prefix, int count, int shown, bool someAreMissing) {
    std::string text;
    if (count == 1) {
        text = std::format("{} {}", count, prefix);
    } else {
        text = std::format("{} {}s", count, prefix);
    }
    if (shown < count) {
        text = std::format("{} of {}", shown, text);
    } else if (someAreMissing && count > 1) {
        text = "all " + text;
    }
    return text;
}

std::string ErrorAndWarningSummary(int errors, int warnings, int shownErrors, int shownWarnings) {
    bool someAreMissing = shownWarnings < warnings || shownErrors < errors;
    if (errors == 0) {
        return Plural("warning", warnings, shownWarnings, someAreMissing);
    } else if (warnings == 0) {
        return Plural("error", errors, shownErrors, someAreMissing);
    } else {
        return std::format("{} and {}",
            Plural("warning", warnings, shownWarnings, someAreMissing),
            Plural("error", errors, shownErrors, someAreMissing));
    }
}


void PrintMessageToStderr(const std::vector<std::string>& os_args, const Msg& msg) {
    auto log = NewStderrLog(OutputOptionsForArgs(os_args));
    log.add_msg(msg);
    log.done();
}

void PrintErrorToStderr(const std::vector<std::string>& os_args, const std::string& text) {
    Msg msg;
    msg.kind = MsgKind::kError;
    msg.data.text = text;
    PrintMessageToStderr(os_args, msg);
}

void PrintErrorWithNoteToStderr(const std::vector<std::string>& os_args, const std::string& text, const std::string& note) {
    Msg msg;
    msg.kind = MsgKind::kError;
    msg.data.text = text;
    if (!note.empty()) {
        MsgData noteData;
        noteData.text = note;
        msg.notes.push_back(std::move(noteData));
    }
    PrintMessageToStderr(os_args, msg);
}

void PrintTextWithColor(int fd, UseColor use_color, const std::function<std::string(const Colors&)>& callback) {
    bool useColorEscapes = false;
    switch (use_color) {
        case UseColor::kColorNever:
            useColorEscapes = false;
            break;
        case UseColor::kColorAlways:
            useColorEscapes = kSupportsColorEscapes;
            break;
        case UseColor::kColorIfTerminal: {
            auto info = GetTerminalInfo(fd);
            useColorEscapes = info.use_color_escapes;
            break;
        }
    }

    Colors colors;

    if (useColorEscapes) {
        colors = kTerminalColors;
    }
    WriteStringWithColor(fd, callback(colors));
}


void PrintText(int fd, LogLevel level, const std::vector<std::string>& os_args, const std::function<std::string(const Colors&)>& callback) {
    auto options = OutputOptionsForArgs(os_args);
    if (options.log_level > level) return;
    PrintTextWithColor(fd, options.color, callback);
}

//------------------------------------------------------------------------------
// Text-এর মধ্যে থাকা HTTPS URL গুলোকে underline style দিয়ে highlight করে।
//
// কাজের ধাপ:
//   1. যদি underline ANSI escape sequence খালি থাকে, তাহলে কোনো পরিবর্তন
//      না করে মূল text-ই ফেরত দেয়।
//   2. text-এর মধ্যে "https://" দিয়ে শুরু হওয়া প্রতিটি URL খুঁজে বের করে।
//   3. URL-এর শেষ পর্যন্ত (পরবর্তী space পর্যন্ত) অংশকে link হিসেবে ধরে।
//   4. URL-এর শেষে '.', ',', '?', '!', ')', ']', '}' ইত্যাদি punctuation
//      থাকলে সেগুলো link-এর অংশ হিসেবে গণনা করা হয় না।
//   5. URL-এর আগে underline escape sequence এবং পরে reset escape sequence
//      যোগ করে নতুন string তৈরি করে।
//   6. সব URL process শেষে সম্পূর্ণ formatted text return করে।
//
// উদ্দেশ্য:
//   Terminal-এ URL গুলোকে visually আলাদা করে দেখানো, যাতে modern terminal
//   emulator-এ link সহজে বোঝা যায় বা clickable link হিসেবে render হতে পারে.
//
// উদাহরণ:
//   Input:
//       "See https://example.com/docs."
//
//   Output:
//       "See <underline>https://example.com/docs<reset>."
std::string LinkifyText(std::string_view text, std::string_view underline, std::string_view reset)
{
    if (underline.empty()) {
        return std::string(text);
    }

    static constexpr std::string_view kHTTPS = "https://";

    auto httpsPos = text.find(kHTTPS);
    if (httpsPos == std::string_view::npos) {
        return std::string(text);
    }

    std::string result;
    std::string_view remaining = text;

    while (true) {
        auto pos = remaining.find(kHTTPS);

        if (pos == std::string_view::npos) {
            break;
        }

        auto end = remaining.find(' ', pos);

        if (end == std::string_view::npos) {
            end = remaining.size();
        }

        if (end > pos) {
            char last = remaining[end - 1];

            if (last == '.' || last == ',' ||
                last == '?' || last == '!' ||
                last == ')' || last == ']' ||
                last == '}') {
                end--;
            }
        }

        result.append(remaining.substr(0, pos));
        result.append(underline);
        result.append(remaining.substr(pos, end - pos));
        result.append(reset);

        remaining.remove_prefix(end);
    }

    result.append(remaining);

    return result;
}

int RuneWidth(char32_t r) {
    (void)r;
    return 1;
}

std::vector<std::string> WrapWordsInString(const std::string& text, int width) {
    std::vector<std::string> runs;

    std::string remaining = text;
    while (!remaining.empty()) {
        size_t i = 0;
        int x = 0;
        size_t wordEndI = 0;

        // Skip leading spaces
        while (i < remaining.size() && remaining[i] == ' ') {
            i++;
            x++;
        }

        // Find how many words fit
        while (i < remaining.size()) {
            size_t oldWordEndI = wordEndI;
            size_t wordStartI = i;

            // Find end of word
            while (i < remaining.size()) {
                auto [c, w] = helpers::DecodeRuneInString(remaining.substr(i));
                if (c == U' ') break;
                i += static_cast<size_t>(w);
                x += RuneWidth(c);
            }
            wordEndI = i;

            if (wordStartI > 0 && x > width) {
                runs.push_back(remaining.substr(0, oldWordEndI));
                remaining = remaining.substr(wordStartI);
                goto nextLine;
            }

            // Skip spaces after word
            while (i < remaining.size() && remaining[i] == ' ') {
                i++;
                x++;
            }
        }

        break;
        nextLine:;
    }

    // Remove trailing spaces
    while (!remaining.empty() && remaining.back() == ' ') {
        remaining.pop_back();
    }
    runs.push_back(remaining);
    return runs;
}


int EstimateWidthInTerminal(const std::string& text) {
    int width = 0;
    for (size_t i = 0; i < text.size(); ) {
        auto [c, w] = helpers::DecodeRuneInString(text.substr(i));
        i += static_cast<size_t>(w);
        if (c != 0xFEFF) {
            width += RuneWidth(c);
        }
    }
    return width;
}


std::string RenderTabStops(const std::string& with_tabs, int spacesPerTab) {
    if (with_tabs.find('\t') == std::string::npos) return with_tabs;

    std::string result;
    int count = 0;

    for (size_t i = 0; i < with_tabs.size(); ) {
        auto [c, w] = helpers::DecodeRuneInString(with_tabs.substr(i));
        i += static_cast<size_t>(w);
        if (c == U'\t') {
            int spaces = spacesPerTab - count % spacesPerTab;
            for (int s = 0; s < spaces; s++) {
                result += ' ';
                count++;
            }
        } else {
            result += static_cast<char>(c);
            count++;
        }
    }

    return result;
}


std::string MarginWithLineText(int maxMargin, int line)
{
    std::string number = std::to_string(line);

    int padding = maxMargin - static_cast<int>(number.size());
    if (padding < 0) {
        padding = 0;
    }

    return "      " +
           std::string(static_cast<size_t>(padding), ' ') +
           number +
           " │ ";
}

std::string EmptyMarginText(int maxMargin, bool isLast)
{
    std::string space(static_cast<size_t>(maxMargin), ' ');

    if (isLast) {
        return "      " + space + " ╵ ";
    }

    return "      " + space + " │ ";
}

bool HasPrefix(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool HasSuffix(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

MsgDetail DetailStruct(const MsgData& data, PathStyle path_style, const TerminalInfo& terminal_info, int max_margin) {
    auto loc = *data.location;
    auto endOfFirstLine = static_cast<int>(loc.line_text.size());
    auto newlinePos = loc.line_text.find('\n');
    if (newlinePos != std::string::npos) {
        endOfFirstLine = static_cast<int>(newlinePos);
    }

    std::string firstLine = loc.line_text.substr(0, static_cast<size_t>(endOfFirstLine));
    std::string afterFirstLine = loc.line_text.substr(static_cast<size_t>(endOfFirstLine));
    if (!afterFirstLine.empty() && !HasSuffix(afterFirstLine, "\n")) {
        afterFirstLine += "\n";
    }

    if (loc.line < 0) loc.line = 0;
    if (loc.column < 0) loc.column = 0;
    if (loc.length < 0) loc.length = 0;
    if (loc.column > endOfFirstLine) loc.column = endOfFirstLine;
    if (loc.length > endOfFirstLine - loc.column) loc.length = endOfFirstLine - loc.column;

    int spacesPerTab = 2;
    std::string lineText = RenderTabStops(firstLine, spacesPerTab);
    std::string textUpToLoc = RenderTabStops(firstLine.substr(0, static_cast<size_t>(loc.column)), spacesPerTab);
    int markerStart = static_cast<int>(textUpToLoc.size());
    int markerEnd = markerStart;
    std::string indent(static_cast<size_t>(EstimateWidthInTerminal(textUpToLoc)), ' ');
    std::string marker = "^";

    if (loc.length > 0) {
        markerEnd = static_cast<int>(RenderTabStops(
            firstLine.substr(0, static_cast<size_t>(loc.column + loc.length)), spacesPerTab).size());
    }

    if (markerStart > static_cast<int>(lineText.size())) markerStart = static_cast<int>(lineText.size());
    if (markerEnd > static_cast<int>(lineText.size())) markerEnd = static_cast<int>(lineText.size());
    if (markerEnd < markerStart) markerEnd = markerStart;

    int width = terminal_info.width;
    if (width < 1) width = kDefaultTerminalWidth;
    width -= max_margin + kExtraMarginChars;
    if (width < 1) width = 1;

    if (loc.column == endOfFirstLine) {
        width -= 1;
    }

    if (static_cast<int>(lineText.size()) > width) {
        int sliceStart = (markerStart + markerEnd - width) / 2;
        if (sliceStart > markerStart - width / 5) {
            sliceStart = markerStart - width / 5;
        }
        if (sliceStart < 0) sliceStart = 0;
        if (sliceStart > static_cast<int>(lineText.size()) - width) {
            sliceStart = static_cast<int>(lineText.size()) - width;
        }
        int sliceEnd = sliceStart + width;

        std::string slicedLine = lineText.substr(static_cast<size_t>(sliceStart), static_cast<size_t>(width));
        markerStart -= sliceStart;
        markerEnd -= sliceStart;
        if (markerStart < 0) markerStart = 0;
        if (markerEnd > static_cast<int>(slicedLine.size())) markerEnd = static_cast<int>(slicedLine.size());

        if (static_cast<int>(slicedLine.size()) > 3 && sliceStart > 0) {
            slicedLine = "..." + slicedLine.substr(3);
            if (markerStart < 3) markerStart = 3;
        }
        if (static_cast<int>(slicedLine.size()) > 3 && sliceEnd < static_cast<int>(lineText.size())) {
            slicedLine = slicedLine.substr(0, slicedLine.size() - 3) + "...";
            if (markerEnd > static_cast<int>(slicedLine.size()) - 3) {
                markerEnd = static_cast<int>(slicedLine.size()) - 3;
            }
            if (markerEnd < markerStart) markerEnd = markerStart;
        }

        lineText = slicedLine;
        indent = std::string(static_cast<size_t>(EstimateWidthInTerminal(lineText.substr(0, static_cast<size_t>(markerStart)))), ' ');
    }

    if (markerEnd - markerStart > 1) {
        marker = std::string(static_cast<size_t>(EstimateWidthInTerminal(lineText.substr(static_cast<size_t>(markerStart), static_cast<size_t>(markerEnd - markerStart)))), '~');
    }

    auto margin = MarginWithLineText(max_margin, loc.line);

    return MsgDetail{
        .source_before = margin + lineText.substr(0, static_cast<size_t>(markerStart)),
        .source_marked = lineText.substr(static_cast<size_t>(markerStart), static_cast<size_t>(markerEnd - markerStart)),
        .source_after = lineText.substr(static_cast<size_t>(markerEnd)),
        .indent = indent,
        .marker = marker,
        .suggestion = loc.suggestion,
        .content_after = afterFirstLine,
        .path = loc.file.Select(path_style),
        .line = loc.line,
        .column = loc.column,
    };
}

std::string MsgString(bool include_source, PathStyle path_style, const TerminalInfo& terminal_info, MsgID id, MsgKind kind, const MsgData& data, const std::string& plugin_name) {
    if (!include_source) {
        if (data.location) {
            return std::format("{}: {}: {}\n",
                data.location->file.Select(path_style), MsgKindToString(kind), data.text);
        }
        return std::format("{}: {}\n", MsgKindToString(kind), data.text);
    }

    Colors colors{};
    if (terminal_info.use_color_escapes) {
        colors = kTerminalColors;
    }

    // Note handling
    if (kind == MsgKind::kNote) {
        std::string noteText;
        std::istringstream stream(data.text);
        std::string line;
        while (std::getline(stream, line)) {
            if (terminal_info.width > 2) {
                int wrapWidth = terminal_info.width;
                if (!data.disable_maximum_width && wrapWidth > 100) {
                    wrapWidth = 100;
                }
                for (const auto& run : WrapWordsInString(line, wrapWidth - 2)) {
                    noteText += "  ";
                    noteText += LinkifyText(run, colors.underline, colors.reset);
                    noteText += '\n';
                }
                continue;
            }
            noteText += "  ";
            noteText += LinkifyText(line, colors.underline, colors.reset);
            noteText += '\n';
        }

        if (data.location) {
            auto detail = DetailStruct(data, path_style, terminal_info, 0);
            if (!detail.suggestion.empty()) {
                noteText += std::format("\n    {}:{}:{}:\n{}{}{}{}{}{}\n{}{}{}{}{}\n{}{}{}{}{}\n{}",
                    detail.path, detail.line, detail.column,
                    colors.dim, detail.source_before, colors.green, detail.source_marked, colors.dim, detail.source_after,
                    EmptyMarginText(0, false), detail.indent, colors.green, detail.marker, colors.dim,
                    EmptyMarginText(0, true), detail.indent, colors.green, detail.suggestion, colors.reset,
                    detail.content_after);
            } else {
                noteText += std::format("\n    {}:{}:{}:\n{}{}{}{}{}{}\n{}{}{}{}{}\n{}",
                    detail.path, detail.line, detail.column,
                    colors.dim, detail.source_before, colors.green, detail.source_marked, colors.dim, detail.source_after,
                    EmptyMarginText(0, true), detail.indent, colors.green, detail.marker, colors.reset,
                    detail.content_after);
            }
        }
        return noteText;
    }

    std::string iconColor;
    std::string kindColorBrackets;
    std::string kindColorText;

    switch (kind) {
        case MsgKind::kVerbose:
            iconColor = colors.cyan;
            kindColorBrackets = colors.cyan_bg_cyan;
            kindColorText = colors.cyan_bg_black;
            break;
        case MsgKind::kDebug:
            iconColor = colors.green;
            kindColorBrackets = colors.green_bg_green;
            kindColorText = colors.green_bg_white;
            break;
        case MsgKind::kInfo:
            iconColor = colors.blue;
            kindColorBrackets = colors.blue_bg_blue;
            kindColorText = colors.blue_bg_white;
            break;
        case MsgKind::kError:
            iconColor = colors.red;
            kindColorBrackets = colors.red_bg_red;
            kindColorText = colors.red_bg_white;
            break;
        case MsgKind::kWarning:
            iconColor = colors.yellow;
            kindColorBrackets = colors.yellow_bg_yellow;
            kindColorText = colors.yellow_bg_black;
            break;
        default: break;
    }

    std::string location;
    if (data.location) {
        int maxMargin = static_cast<int>(std::to_string(data.location->line).size());
        auto d = DetailStruct(data, path_style, terminal_info, maxMargin);

        if (!d.suggestion.empty()) {
            location = std::format("\n    {}:{}:{}:\n{}{}{}{}{}{}\n{}{}{}{}{}\n{}{}{}{}{}\n{}",
                d.path, d.line, d.column,
                colors.dim, d.source_before, colors.green, d.source_marked, colors.dim, d.source_after,
                EmptyMarginText(maxMargin, false), d.indent, colors.green, d.marker, colors.dim,
                EmptyMarginText(maxMargin, true), d.indent, colors.green, d.suggestion, colors.reset,
                d.content_after);
        } else {
            location = std::format("\n    {}:{}:{}:\n{}{}{}{}{}{}\n{}{}{}{}{}\n{}",
                d.path, d.line, d.column,
                colors.dim, d.source_before, colors.green, d.source_marked, colors.dim, d.source_after,
                EmptyMarginText(maxMargin, true), d.indent, colors.green, d.marker, colors.reset,
                d.content_after);
        }
    }

    std::string pluginName;
    if (!plugin_name.empty()) {
        pluginName = std::format(" {}[{}plugin {}]{}", colors.bold, colors.magenta, plugin_name, colors.reset);
    }

    std::string_view msgIDStr = MsgIDToString(id);
    std::string msgIDSuffix;
    if (!msgIDStr.empty()) {
        msgIDSuffix = std::format(" [{}]", msgIDStr);
    }

    return std::format("{}{} {}[{}{}]{} {}{}{}{}{}{}\n{}",
        iconColor, MsgKindToIcon(kind),
        kindColorBrackets, kindColorText, MsgKindToString(kind), kindColorBrackets, colors.reset,
        colors.bold, data.text, colors.reset, pluginName, msgIDSuffix,
        location);
}

// Options এবং terminal তথ্য ব্যবহার করে message-কে formatted string-এ রূপান্তর করে।
std::string Msg::String(const OutputOptions& options, const TerminalInfo& terminal_info) const {
    return MsgString(options.include_source, options.path_style, terminal_info, id, kind, data, plugin_name);
}


//------------------------------------------------------------------------------
// Standard error (stderr) ভিত্তিক logger তৈরি করে।
//
// এই logger-এর উদ্দেশ্য হলো build/process চলাকালীন আসা message গুলো
// thread-safe ভাবে সংগ্রহ, sort এবং terminal-এ প্রদর্শন করা।
//
// কাজের ধাপ:
//   1. stderr terminal-এর তথ্য (TTY, terminal size, color support) সংগ্রহ করে।
//   2. OutputOptions অনুযায়ী color output চালু বা বন্ধ করে।
//   3. সব message অভ্যন্তরীণ list-এ সংরক্ষণ করে।
//   4. LogLevel অনুযায়ী কোন message দেখানো হবে তা নির্ধারণ করে।
//   5. Error ও warning-এর সংখ্যা গণনা করে।
//   6. Message limit অতিক্রম করলে অতিরিক্ত message প্রদর্শন বন্ধ করে।
//   7. Warning গুলো প্রয়োজনে সাময়িকভাবে defer করে রাখে, যাতে পরে
//      কোনো error এলে সেটি দেখানোর জন্য slot খালি থাকে।
//   8. Done() call হলে deferred warning প্রদর্শন করে, message গুলো
//      stable sort করে এবং শেষে summary print করে।
//
// Thread safety:
//   একাধিক thread থেকে AddMsg(), Peek(), Done() অথবা HasErrors()
//   একসাথে call হলেও std::mutex ব্যবহার করে shared state নিরাপদ রাখা হয়.
//
// Go parity:
//   Go-এর NewStderrLog() implementation-এর সমতুল্য C++ port।
Log NewStderrLog(const OutputOptions& options)
{
    auto    state = std::make_shared<StderrLogState>();
    state-> options = options;
    state-> terminal_info = GetTerminalInfo(2);
    state-> remainingMessagesBeforeLimit = options.message_limit > 0 ? options.message_limit : 0x7FFFFFFF;

    if (options.color == UseColor::kColorNever) {
        state->terminal_info.use_color_escapes = false;
    } else if (options.color == UseColor::kColorAlways) {
        state->terminal_info.use_color_escapes = kSupportsColorEscapes;
    }

        auto finalizeLog = [state]() {
        for (auto it = state->deferredWarnings.begin();
             state->remainingMessagesBeforeLimit > 0 && it != state->deferredWarnings.end();) {
            state->shownWarnings++;
            WriteStringWithColor(2, it->String(state->options, state->terminal_info));
            it = state->deferredWarnings.erase(it);
            state->remainingMessagesBeforeLimit--;
        }

        int totalErrors = state->errors;
        int totalWarnings = state->warnings;
        if ((state->options.message_limit > 0 && totalErrors + totalWarnings > state->options.message_limit) ||
            (state->options.log_level <= LogLevel::kInfo && (totalWarnings != 0 || totalErrors != 0))) {
            if (state->options.message_limit > 0 && totalErrors + totalWarnings > state->options.message_limit) {
                WriteStringWithColor(2, std::format("{} shown (disable the message limit with --log-limit=0)\n",
                    ErrorAndWarningSummary(totalErrors, totalWarnings, state->shownErrors, state->shownWarnings)));
            } else if (state->options.log_level <= LogLevel::kInfo && (totalWarnings != 0 || totalErrors != 0)) {
                WriteStringWithColor(2, std::format("{}\n",
                    ErrorAndWarningSummary(totalErrors, totalWarnings, state->shownErrors, state->shownWarnings)));
            }
        }
    };

    Log log;
    log.level = options.log_level;
    log.overrides = options.overrides;

    log.add_msg = [state, finalizeLog](Msg msg) {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->msgs.push_back(msg);

        switch (msg.kind) {
            case MsgKind::kVerbose:
                if (state->options.log_level <= LogLevel::kVerbose) {
                    WriteStringWithColor(2, msg.String(state->options, state->terminal_info));
                }
                break;
            case MsgKind::kDebug:
                if (state->options.log_level <= LogLevel::kDebug) {
                    WriteStringWithColor(2, msg.String(state->options, state->terminal_info));
                }
                break;
            case MsgKind::kInfo:
                if (state->options.log_level <= LogLevel::kInfo) {
                    WriteStringWithColor(2, msg.String(state->options, state->terminal_info));
                }
                break;
            case MsgKind::kError:
                state->hasErrors = true;
                if (state->options.log_level <= LogLevel::kError) {
                    state->errors++;
                }
                break;
            case MsgKind::kWarning:
                if (state->options.log_level <= LogLevel::kWarning) {
                    state->warnings++;
                }
                break;
            default: break;
        }

        if (state->remainingMessagesBeforeLimit == 0) return;

        switch (msg.kind) {
            case MsgKind::kError:
                if (state->options.log_level <= LogLevel::kError) {
                    state->shownErrors++;
                    WriteStringWithColor(2, msg.String(state->options, state->terminal_info));
                    state->remainingMessagesBeforeLimit--;
                }
                break;
            case MsgKind::kWarning:
                if (state->options.log_level <= LogLevel::kWarning) {
                    if (state->remainingMessagesBeforeLimit > (state->options.message_limit + 1) / 2) {
                        state->shownWarnings++;
                        WriteStringWithColor(2, msg.String(state->options, state->terminal_info));
                        state->remainingMessagesBeforeLimit--;
                    } else {
                        state->deferredWarnings.push_back(msg);
                    }
                }
                break;
            default: break;
        }
    };

    log.has_errors = [state]() -> bool {
        std::lock_guard<std::mutex> lock(state->mutex);
        return state->hasErrors;
    };

    log.peek = [state]() -> std::vector<Msg> {
        std::lock_guard<std::mutex> lock(state->mutex);
        std::stable_sort(state->msgs.begin(), state->msgs.end(),
            [](const Msg& a, const Msg& b) {
                if (!a.data.location && !b.data.location) return false;
                if (!a.data.location) return true;
                if (!b.data.location) return false;
                auto& ai = *a.data.location;
                auto& bj = *b.data.location;
                if (ai.file.abs != bj.file.abs) {
                    return ai.file.abs < bj.file.abs ||
                        (ai.file.abs == bj.file.abs && ai.file.rel < bj.file.rel);
                }
                if (ai.line != bj.line) return ai.line < bj.line;
                if (ai.column != bj.column) return ai.column < bj.column;
                if (a.kind != b.kind) return a.kind < b.kind;
                return a.data.text < b.data.text;
            });
        return state->msgs;
    };

    log.done = [state, finalizeLog]() -> std::vector<Msg> {
        std::lock_guard<std::mutex> lock(state->mutex);
        finalizeLog();
        std::stable_sort(state->msgs.begin(), state->msgs.end(),
            [](const Msg& a, const Msg& b) {
                if (!a.data.location && !b.data.location) return false;
                if (!a.data.location) return true;
                if (!b.data.location) return false;
                auto& ai = *a.data.location;
                auto& bj = *b.data.location;
                if (ai.file.abs != bj.file.abs) {
                    return ai.file.abs < bj.file.abs ||
                        (ai.file.abs == bj.file.abs && ai.file.rel < bj.file.rel);
                }
                if (ai.line != bj.line) return ai.line < bj.line;
                if (ai.column != bj.column) return ai.column < bj.column;
                if (a.kind != b.kind) return a.kind < b.kind;
                return a.data.text < b.data.text;
            });
        return std::move(state->msgs);
    };

    return log;
}

Log NewDeferLog(DeferLogKind kind, const std::unordered_map<MsgID, LogLevel>& overrides) {
    auto state = std::make_shared<DeferLogState>();

    Log log;
    log.level = LogLevel::kInfo;
    log.overrides = overrides;

    log.add_msg = [state, kind](Msg msg) {
        if (kind == DeferLogKind::kDeferLogNoVerboseOrDebug &&
            (msg.kind == MsgKind::kVerbose || msg.kind == MsgKind::kDebug)) {
            return;
        }
        std::lock_guard<std::mutex> lock(state->mutex);
        if (msg.kind == MsgKind::kError) {
            state->hasErrors = true;
        }
        state->msgs.push_back(std::move(msg));
    };

    log.has_errors = [state]() -> bool {
        std::lock_guard<std::mutex> lock(state->mutex);
        return state->hasErrors;
    };

    log.peek = [state]() -> std::vector<Msg> {
        std::lock_guard<std::mutex> lock(state->mutex);
        return state->msgs;
    };

    log.done = [state]() -> std::vector<Msg> {
        std::lock_guard<std::mutex> lock(state->mutex);
        std::stable_sort(state->msgs.begin(), state->msgs.end(),
            [](const Msg& a, const Msg& b) {
                if (!a.data.location && !b.data.location) return false;
                if (!a.data.location) return true;
                if (!b.data.location) return false;
                auto& ai = *a.data.location;
                auto& bj = *b.data.location;
                if (ai.file.abs != bj.file.abs) {
                    return ai.file.abs < bj.file.abs ||
                        (ai.file.abs == bj.file.abs && ai.file.rel < bj.file.rel);
                }
                if (ai.line != bj.line) return ai.line < bj.line;
                if (ai.column != bj.column) return ai.column < bj.column;
                if (a.kind != b.kind) return a.kind < b.kind;
                return a.data.text < b.data.text;
            });
        return std::move(state->msgs);
    };

    return log;
}


LineColumnTracker::LineColumnTracker(const Source* source) {
    if (source) {
        contents_ = source->contents;
        pretty_paths_ = source->pretty_paths;
        has_line_start_ = true;
        has_source_ = true;
    }
}

void LineColumnTracker::ScanTo(int32_t offset) {
    auto& contents = contents_;
    int32_t i = offset_;

    if (i < offset) {
        while (true) {
            auto [r, w] = helpers::DecodeRuneInString(contents.substr(static_cast<size_t>(i)));
            i += static_cast<int32_t>(w);

            if (r == U'\n') {
                has_line_start_ = true;
                has_line_end_ = false;
                line_start_ = i;
                if (i == static_cast<int32_t>(w) || contents[static_cast<size_t>(i) - static_cast<size_t>(w) - 1] != '\r') {
                    line_++;
                }
            } else if (r == U'\r' || r == U'\u2028' || r == U'\u2029') {
                has_line_start_ = true;
                has_line_end_ = false;
                line_start_ = i;
                line_++;
            }

            if (i >= offset) {
                offset_ = i;
                return;
            }
        }
    }

    if (i > offset) {
        while (true) {
            auto [r, w] = helpers::DecodeLastRuneInString(std::string_view(contents).substr(0, static_cast<size_t>(i)));
            i -= static_cast<int32_t>(w);

            if (r == U'\n') {
                has_line_start_ = false;
                has_line_end_ = true;
                line_end_ = i;
                if (i == 0 || contents[static_cast<size_t>(i) - 1] != '\r') {
                    line_--;
                }
            } else if (r == U'\r' || r == U'\u2028' || r == U'\u2029') {
                has_line_start_ = false;
                has_line_end_ = true;
                line_end_ = i;
                line_--;
            }

            if (i <= offset) {
                offset_ = i;
                return;
            }
        }
    }
}

void LineColumnTracker::ComputeLineAndColumn(int offset, int32_t& line_count, int32_t& column_count,
                                               int32_t& line_start, int32_t& line_end) {
    ScanTo(static_cast<int32_t>(offset));

    if (!has_line_start_) {
        auto& contents = contents_;
        int32_t i = offset_;
        while (i > 0) {
            auto [r, w] = helpers::DecodeLastRuneInString(std::string_view(contents_).substr(0, static_cast<size_t>(i)));
            if (r == U'\n' || r == U'\r' || r == U'\u2028' || r == U'\u2029') break;
            i -= static_cast<int32_t>(w);
        }
        has_line_start_ = true;
        line_start_ = i;
    }

    if (!has_line_end_) {
        auto& contents = contents_;
        int32_t i = offset_;
        int32_t n = static_cast<int32_t>(contents.size());
        while (i < n) {
            auto [r, w] = helpers::DecodeRuneInString(contents.substr(static_cast<size_t>(i)));
            if (r == U'\n' || r == U'\r' || r == U'\u2028' || r == U'\u2029') break;
            i += static_cast<int32_t>(w);
        }
        has_line_end_ = true;
        line_end_ = i;
    }

    line_count = line_;
    column_count = offset - static_cast<int>(line_start_);
    line_start = static_cast<int>(line_start_);
    line_end = static_cast<int>(line_end_);
}

MsgData LineColumnTracker::MakeMsgData(Range r, const std::string& text) {
    MsgData data;
    data.text = text;
    data.location = MsgLocationOrNil(r);
    return data;
}

std::shared_ptr<MsgLocation> LineColumnTracker::MsgLocationOrNil(Range r) {
    if (!has_source_) return nullptr;

    int32_t lineCount, columnCount, lineStart, lineEnd;
    ComputeLineAndColumn(static_cast<int>(r.loc.start), lineCount, columnCount, lineStart, lineEnd);

    auto loc = std::make_shared<MsgLocation>();
    loc->file = pretty_paths_;
    loc->line = static_cast<int>(lineCount + 1);
    loc->column = static_cast<int>(columnCount);
    loc->length = static_cast<int>(r.len);
    loc->line_text = contents_.substr(static_cast<size_t>(lineStart), static_cast<size_t>(lineEnd - lineStart));
    return loc;
}

void PrintSummary(UseColor use_color, std::vector<SummaryTableEntry>& table, const double* elapsed_ms) {
    PrintTextWithColor(2, use_color, [&](const Colors& colors) -> std::string {
        bool isWinCmd = IsProbablyWindowsCommandPrompt();
        std::string sb;

        if (!table.empty()) {
            TerminalInfo info = GetTerminalInfo(2);

            int maxLength = info.height / 2;
            if (info.height == 0) maxLength = 20;
            else if (maxLength < 5) maxLength = 5;

            int length = static_cast<int>(table.size());

            std::sort(table.begin(), table.end(),
                [](const SummaryTableEntry& a, const SummaryTableEntry& b) {
                    if (!a.is_source_map && b.is_source_map) return true;
                    if (a.is_source_map && !b.is_source_map) return false;
                    if (a.bytes > b.bytes) return true;
                    if (a.bytes < b.bytes) return false;
                    if (a.dir < b.dir) return true;
                    if (a.dir > b.dir) return false;
                    return a.base < b.base;
                });

            if (length > maxLength) {
                table.resize(static_cast<size_t>(maxLength));
            }

            int spacingBetweenColumns = 2;
            bool hasSizeWarning = false;
            int maxPath = 0;
            int maxSize = 0;
            for (const auto& entry : table) {
                int path = static_cast<int>(entry.dir.size() + entry.base.size());
                int size = static_cast<int>(entry.size.size()) + spacingBetweenColumns;
                if (path > maxPath) maxPath = path;
                if (size > maxSize) maxSize = size;
                if (!entry.is_source_map && entry.bytes >= kSizeWarningThreshold) {
                    hasSizeWarning = true;
                }
            }

            std::string margin = "  ";
            int layoutWidth = info.width;
            if (layoutWidth < 1) layoutWidth = kDefaultTerminalWidth;
            layoutWidth -= 2 * static_cast<int>(margin.size());
            if (hasSizeWarning) layoutWidth -= 2;
            if (layoutWidth > maxPath + maxSize) layoutWidth = maxPath + maxSize;

            sb += '\n';

            for (const auto& entry : table) {
                std::string dir = entry.dir;
                std::string base = entry.base;
                int pathWidth = layoutWidth - maxSize;

                if (static_cast<int>(dir.size() + base.size()) > pathWidth) {
                    if (!dir.empty()) {
                        int n = pathWidth - static_cast<int>(base.size()) - 3;
                        if (n < 1) n = 1;
                        dir = "..." + dir.substr(dir.size() - static_cast<size_t>(n));
                    }
                    if (static_cast<int>(dir.size() + base.size()) > pathWidth) {
                        int n = pathWidth - static_cast<int>(dir.size()) - 3;
                        if (n < 0) n = 0;
                        base = base.substr(0, static_cast<size_t>(n)) + "...";
                    }
                }

                int spacer = layoutWidth - static_cast<int>(entry.size.size() + dir.size() + base.size());
                if (spacer < 0) spacer = 0;

                std::string_view sizeColor = colors.cyan;
                std::string sizeWarning;
                if (!entry.is_source_map && entry.bytes >= kSizeWarningThreshold) {
                    sizeColor = colors.yellow;
                    if (!isWinCmd) {
                        sizeWarning = " \xe2\x9a\xa0\xef\xb8\x8f";
                    }
                }

                sb += std::format("{}{}{}{}{}{}{}{}{}{}{}{}\n",
                    margin,
                    colors.dim, dir, colors.reset,
                    colors.bold, base, colors.reset,
                    std::string(static_cast<size_t>(spacer), ' '),
                    sizeColor, entry.size, sizeWarning, colors.reset);
            }

            if (length > maxLength) {
                std::string plural_s = (length == maxLength + 1) ? "" : "s";
                sb += std::format("{}{}...and {} more output file{}...{}\n",
                    margin, colors.dim, length - maxLength, plural_s, colors.reset);
            }
        }

        sb += '\n';

        std::string lightningSymbol = "\xe2\x9a\xa1 ";
        if (isWinCmd) lightningSymbol = "";

        if (elapsed_ms) {
            int ms = static_cast<int>(*elapsed_ms);
            sb += std::format("{}{}Done in {}ms{}\n",
                lightningSymbol, colors.green, ms, colors.reset);
        }

        return sb;
    });
}


// String-in-JS table functions


std::vector<StringInJSTableEntry> GenerateStringInJSTable(
    const std::string& outer_contents, Loc outer_string_literal_loc,
    const std::string& inner_contents) {
    std::vector<StringInJSTableEntry> table;
    int32_t i = 0;
    int32_t n = static_cast<int32_t>(inner_contents.size());
    int32_t line = 1;
    int32_t column = 0;
    Loc loc{static_cast<int32_t>(outer_string_literal_loc.start + 1)};

    while (i < n) {
        for (;;) {
            if (auto [c, w] = helpers::DecodeRuneInString(outer_contents.substr(static_cast<size_t>(loc.start))); c != U'\\') break;
            auto [c, w] = helpers::DecodeRuneInString(outer_contents.substr(static_cast<size_t>(loc.start + 1)));
            if (c == U'\n' || c == U'\r' || c == U'\u2028' || c == U'\u2029') {
                loc.start += 1 + static_cast<int32_t>(w);
                if (c == U'\r' && outer_contents[static_cast<size_t>(loc.start)] == '\n') {
                    loc.start++;
                }
                continue;
            }
            break;
        }

        auto [c, w] = helpers::DecodeRuneInString(inner_contents.substr(static_cast<size_t>(i)));

        table.push_back({line, column, Loc{i}, loc});
        if (table.size() > 1) {
            auto& last = table[table.size() - 2];
            if (line == last.inner_line && loc.start - column == last.outer_loc.start - last.inner_column) {
                table.pop_back();
            }
        }

        switch (c) {
            case U'\n':
            case U'\r':
            case U'\u2028':
            case U'\u2029':
                line++;
                column = 0;
                if (c == U'\r' && i + 1 < n && inner_contents[static_cast<size_t>(i + 1)] == '\n') {
                    i++;
                }
                break;
            default:
                column += static_cast<int32_t>(w);
                break;
        }
        i += static_cast<int32_t>(w);

        auto [oc, ow] = helpers::DecodeRuneInString(outer_contents.substr(static_cast<size_t>(loc.start)));
        if (oc == U'\r' && outer_contents[static_cast<size_t>(loc.start + 1)] == '\n') {
            loc.start += 2;
        } else if (oc != U'\\') {
            loc.start += static_cast<int32_t>(ow);
        } else {
            auto [ec, ew] = helpers::DecodeRuneInString(outer_contents.substr(static_cast<size_t>(loc.start + 1)));
            switch (ec) {
                case U'x': loc.start += 1 + 2; break;
                case U'u':
                    loc.start++;
                    if (outer_contents[static_cast<size_t>(loc.start)] == '{') {
                        loc.start++;
                        while (outer_contents[static_cast<size_t>(loc.start)] != '}') loc.start++;
                        loc.start++;
                    } else {
                        loc.start += 4;
                    }
                    break;
                case U'\n':
                case U'\r':
                case U'\u2028':
                case U'\u2029':
                    break;
                default:
                    loc.start += 1 + static_cast<int32_t>(ew);
                    break;
            }
        }
    }

    return table;
}

Loc RemapStringInJSLoc(const std::vector<StringInJSTableEntry>& table, Loc inner_loc) {
    int count = static_cast<int>(table.size());
    int index = 0;

    while (count > 0) {
        int step = count / 2;
        int i = index + step;
        if (i + 1 < static_cast<int>(table.size())) {
            if (table[static_cast<size_t>(i + 1)].inner_loc.start < inner_loc.start) {
                index = i + 1;
                count -= step + 1;
                continue;
            }
        }
        count = step;
    }

    auto entry = table[static_cast<size_t>(index)];
    entry.outer_loc.start += inner_loc.start - entry.inner_loc.start;
    return entry.outer_loc;
}

Log NewStringInJSLog(Log log, LineColumnTracker& outer_tracker, const std::vector<StringInJSTableEntry>& table) {
    auto oldAddMsg = log.add_msg;

    auto remapLineAndColumnToLoc = [&](int32_t line, int32_t column) -> Loc {
        int count = static_cast<int>(table.size());
        int index = 0;

        while (count > 0) {
            int step = count / 2;
            int i = index + step;
            if (i + 1 < static_cast<int>(table.size())) {
                const auto& entry = table[static_cast<size_t>(i + 1)];
                if (entry.inner_line < line || (entry.inner_line == line && entry.inner_column < column)) {
                    index = i + 1;
                    count -= step + 1;
                    continue;
                }
            }
            count = step;
        }

        auto entry = table[static_cast<size_t>(index)];
        entry.outer_loc.start += column - entry.inner_column;
        return entry.outer_loc;
    };

    auto remapData = [&](const MsgData& data) -> MsgData {
        if (!data.location) return data;

        Range r;
        r.loc = remapLineAndColumnToLoc(static_cast<int32_t>(data.location->line),
                                         static_cast<int32_t>(data.location->column));
        if (data.location->length != 0) {
            r.len = remapLineAndColumnToLoc(static_cast<int32_t>(data.location->line),
                                             static_cast<int32_t>(data.location->column + data.location->length)).start
                    - r.loc.start;
        }

        MsgData result = data;
        result.location = outer_tracker.MakeMsgData(r, data.text).location;
        if (result.location) {
            result.location->suggestion = data.location->suggestion;
        }
        return result;
    };

    log.add_msg = [oldAddMsg, remapData](Msg msg) {
        msg.data = remapData(msg.data);
        for (auto& note : msg.notes) {
            note = remapData(note);
        }
        oldAddMsg(msg);
    };

    return log;
}

}






