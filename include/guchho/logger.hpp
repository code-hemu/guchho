#pragma once
#include <functional>
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>
#include <unordered_map>

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

struct Msg
{

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


// message.cpp
void             StringToMsgIDs(std::string_view str, LogLevel logLevel, std::unordered_map<MsgID, LogLevel>& overrides);
std::string_view MsgIDToString(MsgID id);
MsgID            StringToMaximumMsgID(std::string_view id);





}






