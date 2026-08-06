#pragma once
#include <functional>
#include <vector>
#include <string>
#include <mutex>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <memory>

namespace guchho::logger {

constexpr bool kSupportsColorEscapes = true;
constexpr int  kDefaultTerminalWidth = 80;
constexpr int  kExtraMarginChars = 9;
constexpr int  kSizeWarningThreshold = 1024 * 1024;

enum class LogLevel : int8_t {
    kNone,
    kVerbose,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kSilent,
};

// বেশিরভাগ log message-এর একটি Message ID থাকে। 
// এই ID দিয়ে message-এর log level (info, warning ইত্যাদি) পরিবর্তন করা যায়. 
// Error message-এর কোনো Message ID নেই, 
// কারণ error-কে warning বা info করলে build ভুলভাবে সফল হতে পারে. 
// Debug বা internal message-এরও কোনো Message ID থাকে না. 
// এসব ক্ষেত্রে kNone ব্যবহার করা হয়.
enum class MsgID : uint8_t {
    kNone,

    // JavaScript
    kJS_AssertToWith,
    kJS_AssertTypeJSON,
    kJS_AssignToConstant,
    kJS_AssignToDefine,
    kJS_AssignToImport,
    kJS_BigInt,
    kJS_CallImportNamespace,
    kJS_ClassNameWillThrow,
    kJS_CommonJSVariableInESM,
    kJS_DeleteSuperProperty,
    kJS_DirectEval,
    kJS_DuplicateCase,
    kJS_DuplicateClassMember,
    kJS_DuplicateObjectKey,
    kJS_EmptyImportMeta,
    kJS_EqualsNaN,
    kJS_EqualsNegativeZero,
    kJS_EqualsNewObject,
    kJS_HTMLCommentInJS,
    kJS_ImpossibleTypeof,
    kJS_IndirectRequire,
    kJS_PrivateNameWillThrow,
    kJS_SemicolonAfterReturn,
    kJS_SuspiciousBooleanNot,
    kJS_SuspiciousDefine,
    kJS_SuspiciousLogicalOperator,
    kJS_SuspiciousNullishCoalescing,
    kJS_ThisIsUndefinedInESM,
    kJS_UnsupportedDynamicImport,
    kJS_UnsupportedJSXComment,
    kJS_UnsupportedRegExp,
    kJS_UnsupportedRequireCall,

    // CSS
    kCSS_CSSSyntaxError,
    kCSS_InvalidAtCharset,
    kCSS_InvalidAtImport,
    kCSS_InvalidAtLayer,
    kCSS_InvalidCalc,
    kCSS_JSCommentInCSS,
    kCSS_UndefinedComposesFrom,
    kCSS_UnsupportedAtCharset,
    kCSS_UnsupportedAtNamespace,
    kCSS_UnsupportedCSSProperty,
    kCSS_UnsupportedCSSNesting,

    // Bundler
    kBundler_AmbiguousReexport,
    kBundler_DifferentPathCase,
    kBundler_EmptyGlob,
    kBundler_IgnoredBareImport,
    kBundler_IgnoredDynamicImport,
    kBundler_ImportIsUndefined,
    kBundler_RequireResolveNotExternal,

    // Source maps
    kSourceMap_InvalidSourceMappings,
    kSourceMap_MissingSourceMap,
    kSourceMap_UnsupportedSourceMapComment,

    // package.json
    kPackageJSON_FIRST,
    kPackageJSON_DeadCondition,
    kPackageJSON_InvalidBrowser,
    kPackageJSON_InvalidImportsOrExports,
    kPackageJSON_InvalidSideEffects,
    kPackageJSON_InvalidType,
    kPackageJSON_LAST,

    // tsconfig.json
    kTSConfigJSON_FIRST,
    kTSConfigJSON_Cycle,
    kTSConfigJSON_InvalidImportsNotUsedAsValues,
    kTSConfigJSON_InvalidJSX,
    kTSConfigJSON_InvalidPaths,
    kTSConfigJSON_InvalidTarget,
    kTSConfigJSON_InvalidTopLevelOption,
    kTSConfigJSON_Missing,
    kTSConfigJSON_LAST,

    kEND,
};

// Log message-এর ধরন।
enum class MsgKind : uint8_t {
    kError,    // Error
    kWarning,  // Warning
    kInfo,     // Information
    kNote,     // অতিরিক্ত তথ্য
    kDebug,    // Debug তথ্য
    kVerbose,  // বিস্তারিত Debug তথ্য
};

enum class PathFlags : uint8_t {
    kPathDisabled = 1 << 0,
};

// Log message কখন defer (delay করে রাখা) হবে তা নির্ধারণ করে।
enum class DeferLogKind : uint8_t {
    // সব ধরনের log message defer করবে।
    kDeferLogAll,

    // Verbose এবং Debug level-এর log defer করবে না।
    // শুধুমাত্র গুরুত্বপূর্ণ log message defer করবে।
    kDeferLogNoVerboseOrDebug,
};

struct Loc {
    // ফাইলের শুরু থেকে byte-এ 0-based অবস্থান।
    int32_t start{};
};


//Source file-এর একটি range (byte offset + length)
struct Range {
    Loc     loc{};
    int32_t len{};

    // Range-এর শেষ byte offset ফেরত দেয়।
    int32_t End() const;

    // আরেকটি range অন্তর্ভুক্ত করার জন্য range প্রসারিত করে।
    void    ExpandBy(const Range& b);
};

// Path display style.
enum class PathStyle : uint8_t {
    kRelPath,
    kAbsPath,
};

// Absolute এবং relative path সংরক্ষণ করে।
struct PrettyPaths {
    std::string abs;
    std::string rel;

    // নির্বাচিত style-এর path ফেরত দেয়।
    std::string Select(PathStyle style) const;
};

struct MsgLocation {
    PrettyPaths file;
    std::string namespace_;
    std::string line_text;
    std::string suggestion;
    int         line{};
    int         column{};
    int         length{};
};

// Terminal output-এ color ব্যবহার করার নিয়ম।
enum class UseColor : uint8_t {
    kColorIfTerminal, // Terminal হলে color ব্যবহার করবে।
    kColorNever,      // Color output বন্ধ থাকবে।
    kColorAlways,     // সব output-এ color ব্যবহার করবে।
};

struct OutputOptions {
    int                                 message_limit{};
    bool                                include_source{};
    UseColor                            color{};
    LogLevel                            log_level{};
    PathStyle                           path_style{};
    std::unordered_map<MsgID, LogLevel> overrides;
};

struct MsgData {
    void*                        user_detail{};
    std::shared_ptr<MsgLocation> location;
    std::string                  text;
    bool                         disable_maximum_width{};
};

// Terminal-এর capability এবং display information সংরক্ষণ করে।
struct TerminalInfo {
    bool is_tty{};              // Output terminal কিনা।
    bool use_color_escapes{};   // ANSI color escape code ব্যবহার করা যাবে কিনা।
    int width{};                // Terminal-এর প্রস্থ (columns)।
    int height{};               // Terminal-এর উচ্চতা (rows)।
};

struct Msg {
    std::vector<MsgData> notes;
    std::string plugin_name;
    MsgData data;
    MsgKind kind{};
    MsgID id{};

    std::string String(const OutputOptions& options, const TerminalInfo& terminal_info) const;
};


struct Log
{
    std::function<void(const Msg&)>     add_msg;
    std::function<bool()>               has_errors;
    std::function<std::vector<Msg>()>   peek;
    std::function<std::vector<Msg>()>   done;
    LogLevel                            level;
    std::unordered_map<MsgID, LogLevel> overrides;

    void AddError(class LineColumnTracker* tracker, Range r, const std::string& text);
    void AddID(MsgID id, MsgKind kind, class LineColumnTracker* tracker, Range r, const std::string& text);
    void AddErrorWithNotes(class LineColumnTracker* tracker, Range r, const std::string& text, const std::vector<MsgData>& notes);
    void AddIDWithNotes(MsgID id, MsgKind kind, class LineColumnTracker* tracker, Range r, const std::string& text, const std::vector<MsgData>& notes);
    void AddMsgID(MsgID id, const Msg& msg);
};

struct ImportAttribute {
    std::string key;
    std::string value;
};

struct ImportAttributes {
    std::string                                  packed_data;
    std::vector<ImportAttribute>                 DecodeIntoArray() const;
    std::unordered_map<std::string, std::string> DecodeIntoMap() const;
};

struct Path {
    std::string      text;
    std::string      namespace_;
    std::string      ignored_suffix;
    ImportAttributes import_attributes;
    PathFlags        flags{};
    bool             IsDisabled() const;
};

struct Source {
    PrettyPaths pretty_paths;
    std::string identifier_name;
    std::string contents;
    Path        key_path;
    uint32_t    index{};

    std::string TextForRange(Range r) const;
    Loc         LocBeforeWhitespace(Loc loc) const;
    Range       RangeOfOperatorBefore(Loc loc, const std::string& op) const;
    Range       RangeOfOperatorAfter(Loc loc, const std::string& op) const;
    Range       RangeOfString(Loc loc) const;
    Range       RangeOfNumber(Loc loc) const;
    Range       RangeOfLegacyOctalEscape(Loc loc) const;
    std::string CommentTextWithoutIndent(const Range& r) const;
};


struct LineColumnTracker {
    // Constructor
    LineColumnTracker() = default;
    explicit LineColumnTracker(const Source* source);

    // Public API
    MsgData MakeMsgData(Range r, const std::string& text);
    std::shared_ptr<MsgLocation> MsgLocationOrNil(Range r);

private:
    // Source content
    std::string contents_;
    PrettyPaths pretty_paths_;

    // Current scan position
    int32_t offset_{0};
    int32_t line_{0};
    int32_t line_start_{0};
    int32_t line_end_{0};

    // Cached state
    bool has_line_start_{false};
    bool has_line_end_{false};
    bool has_source_{false};

    // Internal helpers
    void ScanTo(int32_t offset);

    void ComputeLineAndColumn(
        int offset,
        int32_t& line_count,
        int32_t& column_count,
        int32_t& line_start,
        int32_t& line_end);
};

struct MsgDetail {
    std::string source_before;
    std::string source_marked;
    std::string source_after;
    std::string indent;
    std::string marker;
    std::string suggestion;
    std::string content_after;
    std::string path;
    int         line{};
    int         column{};
};

struct StderrLogState {
    std::mutex       mutex;
    std::vector<Msg> msgs;
    TerminalInfo     terminal_info;
    OutputOptions    options;
    int              errors = 0;
    int              warnings = 0;
    int              shownErrors = 0;
    int              shownWarnings = 0;
    bool             hasErrors = false;
    int              remainingMessagesBeforeLimit = 0x7FFFFFFF;
    std::vector<Msg> deferredWarnings;
};

struct DeferLogState {
    std::mutex       mutex;
    std::vector<Msg> msgs;
    bool             hasErrors = false;
};


struct SummaryTableEntry {
    std::string dir;
    std::string base;
    std::string size;
    int         bytes{};
    bool        is_source_map{};
};

struct StringInJSTableEntry {
    int32_t inner_line{};
    int32_t inner_column{};
    Loc     inner_loc;
    Loc     outer_loc;
};


struct Colors {
    std::string_view reset;
    std::string_view bold;
    std::string_view dim;
    std::string_view underline;

    std::string_view red;
    std::string_view green;
    std::string_view blue;

    std::string_view cyan;
    std::string_view magenta;
    std::string_view yellow;

    std::string_view red_bg_red;
    std::string_view red_bg_white;
    std::string_view green_bg_green;
    std::string_view green_bg_white;
    std::string_view blue_bg_blue;
    std::string_view blue_bg_white;

    std::string_view cyan_bg_cyan;
    std::string_view cyan_bg_black;
    std::string_view magenta_bg_magenta;
    std::string_view magenta_bg_black;
    std::string_view yellow_bg_yellow;
    std::string_view yellow_bg_black;
};

inline constexpr Colors kTerminalColors{
    .reset = "\033[0m",
    .bold = "\033[1m",
    .dim = "\033[37m",
    .underline = "\033[4m",

    .red = "\033[31m",
    .green = "\033[32m",
    .blue = "\033[34m",

    .cyan = "\033[36m",
    .magenta = "\033[35m",
    .yellow = "\033[33m",

    .red_bg_red = "\033[41;31m",
    .red_bg_white = "\033[41;97m",
    .green_bg_green = "\033[42;32m",
    .green_bg_white = "\033[42;97m",
    .blue_bg_blue = "\033[44;34m",
    .blue_bg_white = "\033[44;97m",

    .cyan_bg_cyan = "\033[46;36m",
    .cyan_bg_black = "\033[46;30m",
    .magenta_bg_magenta = "\033[45;35m",
    .magenta_bg_black = "\033[45;30m",
    .yellow_bg_yellow = "\033[43;33m",
    .yellow_bg_black = "\033[43;30m",
};


ImportAttributes EncodeImportAttributes(const std::unordered_map<std::string, std::string>& value);

// logger.cpp
Log NewStringInJSLog(Log log, LineColumnTracker& outer_tracker, const std::vector<StringInJSTableEntry>& table);
Log NewStderrLog(const OutputOptions& options);
Log NewDeferLog(DeferLogKind kind, const std::unordered_map<MsgID, LogLevel>& overrides);

void             PlatformIndependentPathDirBaseExt(const std::string& path, std::string& dir, std::string& base, std::string& ext);
MsgKind          AllowOverride(const std::unordered_map<MsgID, LogLevel>& overrides, MsgID id, MsgKind kind);
OutputOptions    OutputOptionsForArgs(const std::vector<std::string>& os_args);


void PrintErrorToStderr(const std::vector<std::string>& os_args, const std::string& text);
void PrintErrorWithNoteToStderr(const std::vector<std::string>& os_args, const std::string& text, const std::string& note);
void PrintMessageToStderr(const std::vector<std::string>& os_args, const Msg& msg);
void PrintText(int fd, LogLevel level, const std::vector<std::string>& os_args, const std::function<std::string(const Colors&)>& callback);
void PrintTextWithColor(int fd, UseColor use_color, const std::function<std::string(const Colors&)>& callback);

// logger.cpp -> Formatting 
std::string              MsgString(bool include_source, PathStyle path_style, const TerminalInfo& terminal_info, MsgID id, MsgKind kind, const MsgData& data, const std::string& plugin_name);
MsgDetail                DetailStruct(const MsgData& data, PathStyle path_style, const TerminalInfo& terminal_info, int max_margin);
std::string              LinkifyText(std::string_view text, std::string_view underline, std::string_view reset);
std::vector<std::string> WrapWordsInString(const std::string& text, int width);
int                      EstimateWidthInTerminal(const std::string& text);
std::string              RenderTabStops(const std::string& with_tabs, int spaces_per_tab);


// terminal.cpp
TerminalInfo     GetTerminalInfo(int file_descriptor);
bool             HasEnvironmentVariableValue(std::string_view name);
bool             IsProbablyWindowsCommandPrompt();
void             WriteStringWithColor(int fd, const std::string& text);

// message.cpp
void             StringToMsgIDs(std::string_view str, LogLevel logLevel, std::unordered_map<MsgID, LogLevel>& overrides);
std::string_view MsgIDToString(MsgID id);
MsgID            StringToMaximumMsgID(std::string_view id);



}






