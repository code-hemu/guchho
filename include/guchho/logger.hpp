#pragma once
#include <functional>
#include <vector>
#include <string>
// #include <cstdlib>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <memory>



namespace guchho::logger {

    
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
    std::string CommentTextWithoutIndent(Range r) const;
};

ImportAttributes EncodeImportAttributes(const std::unordered_map<std::string, std::string>& value);



// terminal.cpp
TerminalInfo     GetTerminalInfo(int file_descriptor);
bool             HasEnvironmentVariableValue(std::string_view name);
bool             IsProbablyWindowsCommandPrompt();


// message.cpp
void             StringToMsgIDs(std::string_view str, LogLevel logLevel, std::unordered_map<MsgID, LogLevel>& overrides);
std::string_view MsgIDToString(MsgID id);
MsgID            StringToMaximumMsgID(std::string_view id);




}






