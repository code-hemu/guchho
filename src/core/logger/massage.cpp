#include "guchho/logger.hpp"

namespace guchho::logger {

// একটি message-এর নাম (যেমন "assert-to-with" বা "package.json")
// থেকে এক বা একাধিক Message ID খুঁজে বের করে এবং
// সেগুলোর log level `overrides` map-এ সংরক্ষণ করে.
void StringToMsgIDs(std::string_view str, LogLevel logLevel, std::unordered_map<MsgID, LogLevel>& overrides)
{
#define MATCH(s, e) if (str == s) { overrides[MsgID::e] = logLevel; } else

    // JavaScript
    MATCH("assert-to-with",                 kJS_AssertToWith)
    MATCH("assert-type-json",               kJS_AssertTypeJSON)
    MATCH("assign-to-constant",             kJS_AssignToConstant)
    MATCH("assign-to-define",               kJS_AssignToDefine)
    MATCH("assign-to-import",               kJS_AssignToImport)
    MATCH("bigint",                         kJS_BigInt)
    MATCH("call-import-namespace",          kJS_CallImportNamespace)
    MATCH("class-name-will-throw",          kJS_ClassNameWillThrow)
    MATCH("commonjs-variable-in-esm",       kJS_CommonJSVariableInESM)
    MATCH("delete-super-property",          kJS_DeleteSuperProperty)
    MATCH("direct-eval",                    kJS_DirectEval)
    MATCH("duplicate-case",                 kJS_DuplicateCase)
    MATCH("duplicate-class-member",         kJS_DuplicateClassMember)
    MATCH("duplicate-object-key",           kJS_DuplicateObjectKey)
    MATCH("empty-import-meta",              kJS_EmptyImportMeta)
    MATCH("equals-nan",                     kJS_EqualsNaN)
    MATCH("equals-negative-zero",           kJS_EqualsNegativeZero)
    MATCH("equals-new-object",              kJS_EqualsNewObject)
    MATCH("html-comment-in-js",             kJS_HTMLCommentInJS)
    MATCH("impossible-typeof",              kJS_ImpossibleTypeof)
    MATCH("indirect-require",               kJS_IndirectRequire)
    MATCH("private-name-will-throw",        kJS_PrivateNameWillThrow)
    MATCH("semicolon-after-return",         kJS_SemicolonAfterReturn)
    MATCH("suspicious-boolean-not",         kJS_SuspiciousBooleanNot)
    MATCH("suspicious-define",              kJS_SuspiciousDefine)
    MATCH("suspicious-logical-operator",    kJS_SuspiciousLogicalOperator)
    MATCH("suspicious-nullish-coalescing",  kJS_SuspiciousNullishCoalescing)
    MATCH("this-is-undefined-in-esm",       kJS_ThisIsUndefinedInESM)
    MATCH("unsupported-dynamic-import",     kJS_UnsupportedDynamicImport)
    MATCH("unsupported-jsx-comment",        kJS_UnsupportedJSXComment)
    MATCH("unsupported-regexp",             kJS_UnsupportedRegExp)
    MATCH("unsupported-require-call",       kJS_UnsupportedRequireCall)

    // CSS
    MATCH("css-syntax-error",               kCSS_CSSSyntaxError)
    MATCH("invalid-@charset",               kCSS_InvalidAtCharset)
    MATCH("invalid-@import",                kCSS_InvalidAtImport)
    MATCH("invalid-@layer",                 kCSS_InvalidAtLayer)
    MATCH("invalid-calc",                   kCSS_InvalidCalc)
    MATCH("js-comment-in-css",              kCSS_JSCommentInCSS)
    MATCH("undefined-composes-from",        kCSS_UndefinedComposesFrom)
    MATCH("unsupported-@charset",           kCSS_UnsupportedAtCharset)
    MATCH("unsupported-@namespace",         kCSS_UnsupportedAtNamespace)
    MATCH("unsupported-css-property",       kCSS_UnsupportedCSSProperty)
    MATCH("unsupported-css-nesting",        kCSS_UnsupportedCSSNesting)

    // Bundler
    MATCH("ambiguous-reexport",             kBundler_AmbiguousReexport)
    MATCH("different-path-case",            kBundler_DifferentPathCase)
    MATCH("empty-glob",                     kBundler_EmptyGlob)
    MATCH("ignored-bare-import",            kBundler_IgnoredBareImport)
    MATCH("ignored-dynamic-import",         kBundler_IgnoredDynamicImport)
    MATCH("import-is-undefined",            kBundler_ImportIsUndefined)
    MATCH("require-resolve-not-external",   kBundler_RequireResolveNotExternal)

    // Source maps
    MATCH("invalid-source-mappings",        kSourceMap_InvalidSourceMappings)
    MATCH("missing-source-map",             kSourceMap_MissingSourceMap)
    MATCH("unsupported-source-map-comment", kSourceMap_UnsupportedSourceMapComment)

    if (str == "package.json") {
        for (uint8_t i = static_cast<uint8_t>(MsgID::kPackageJSON_FIRST);
             i <= static_cast<uint8_t>(MsgID::kPackageJSON_LAST); ++i) {
            overrides[static_cast<MsgID>(i)] = logLevel;
        }
    }

    else if (str == "tsconfig.json") {
        for (uint8_t i = static_cast<uint8_t>(MsgID::kTSConfigJSON_FIRST);
             i <= static_cast<uint8_t>(MsgID::kTSConfigJSON_LAST); ++i) {
            overrides[static_cast<MsgID>(i)] = logLevel;
        }
    }

    else {
        // অজানা বা অবৈধ Message ID উপেক্ষা করো.
        // কারণ এই কোড লেখার পর Message ID-এর নাম
        // পরিবর্তন বা মুছে ফেলা হয়ে থাকতে পারে.
    }

#undef MATCH
}


// একটি Message ID-কে তার string নাম-এ রূপান্তর করে.
// যেমন: kJS_AssertToWith => "assert-to-with".
std::string_view MsgIDToString(MsgID id)
{
    switch (id) {
        // JavaScript
        case MsgID::kJS_AssertToWith:               return "assert-to-with";
        case MsgID::kJS_AssertTypeJSON:             return "assert-type-json";
        case MsgID::kJS_AssignToConstant:           return "assign-to-constant";
        case MsgID::kJS_AssignToDefine:             return "assign-to-define";
        case MsgID::kJS_AssignToImport:             return "assign-to-import";
        case MsgID::kJS_BigInt:                     return "bigint";
        case MsgID::kJS_CallImportNamespace:        return "call-import-namespace";
        case MsgID::kJS_ClassNameWillThrow:         return "class-name-will-throw";
        case MsgID::kJS_CommonJSVariableInESM:      return "commonjs-variable-in-esm";
        case MsgID::kJS_DeleteSuperProperty:        return "delete-super-property";
        case MsgID::kJS_DirectEval:                 return "direct-eval";
        case MsgID::kJS_DuplicateCase:              return "duplicate-case";
        case MsgID::kJS_DuplicateClassMember:       return "duplicate-class-member";
        case MsgID::kJS_DuplicateObjectKey:         return "duplicate-object-key";
        case MsgID::kJS_EmptyImportMeta:            return "empty-import-meta";
        case MsgID::kJS_EqualsNaN:                  return "equals-nan";
        case MsgID::kJS_EqualsNegativeZero:         return "equals-negative-zero";
        case MsgID::kJS_EqualsNewObject:            return "equals-new-object";
        case MsgID::kJS_HTMLCommentInJS:            return "html-comment-in-js";
        case MsgID::kJS_ImpossibleTypeof:           return "impossible-typeof";
        case MsgID::kJS_IndirectRequire:            return "indirect-require";
        case MsgID::kJS_PrivateNameWillThrow:       return "private-name-will-throw";
        case MsgID::kJS_SemicolonAfterReturn:       return "semicolon-after-return";
        case MsgID::kJS_SuspiciousBooleanNot:       return "suspicious-boolean-not";
        case MsgID::kJS_SuspiciousDefine:           return "suspicious-define";
        case MsgID::kJS_SuspiciousLogicalOperator:  return "suspicious-logical-operator";
        case MsgID::kJS_SuspiciousNullishCoalescing:return "suspicious-nullish-coalescing";
        case MsgID::kJS_ThisIsUndefinedInESM:       return "this-is-undefined-in-esm";
        case MsgID::kJS_UnsupportedDynamicImport:   return "unsupported-dynamic-import";
        case MsgID::kJS_UnsupportedJSXComment:      return "unsupported-jsx-comment";
        case MsgID::kJS_UnsupportedRegExp:          return "unsupported-regexp";
        case MsgID::kJS_UnsupportedRequireCall:     return "unsupported-require-call";

        // CSS
        case MsgID::kCSS_CSSSyntaxError:            return "css-syntax-error";
        case MsgID::kCSS_InvalidAtCharset:          return "invalid-@charset";
        case MsgID::kCSS_InvalidAtImport:           return "invalid-@import";
        case MsgID::kCSS_InvalidAtLayer:            return "invalid-@layer";
        case MsgID::kCSS_InvalidCalc:               return "invalid-calc";
        case MsgID::kCSS_JSCommentInCSS:            return "js-comment-in-css";
        case MsgID::kCSS_UndefinedComposesFrom:     return "undefined-composes-from";
        case MsgID::kCSS_UnsupportedAtCharset:      return "unsupported-@charset";
        case MsgID::kCSS_UnsupportedAtNamespace:    return "unsupported-@namespace";
        case MsgID::kCSS_UnsupportedCSSProperty:    return "unsupported-css-property";
        case MsgID::kCSS_UnsupportedCSSNesting:     return "unsupported-css-nesting";

        // Bundler
        case MsgID::kBundler_AmbiguousReexport:         return "ambiguous-reexport";
        case MsgID::kBundler_DifferentPathCase:         return "different-path-case";
        case MsgID::kBundler_EmptyGlob:                 return "empty-glob";
        case MsgID::kBundler_IgnoredBareImport:         return "ignored-bare-import";
        case MsgID::kBundler_IgnoredDynamicImport:      return "ignored-dynamic-import";
        case MsgID::kBundler_ImportIsUndefined:         return "import-is-undefined";
        case MsgID::kBundler_RequireResolveNotExternal: return "require-resolve-not-external";

        // Source maps
        case MsgID::kSourceMap_InvalidSourceMappings:      return "invalid-source-mappings";
        case MsgID::kSourceMap_MissingSourceMap:           return "missing-source-map";
        case MsgID::kSourceMap_UnsupportedSourceMapComment:return "unsupported-source-map-comment";

        default:
            break;
    }

    if (id >= MsgID::kPackageJSON_FIRST && id <= MsgID::kPackageJSON_LAST) {
        return "package.json";
    }

    if (id >= MsgID::kTSConfigJSON_FIRST && id <= MsgID::kTSConfigJSON_LAST) {
        return "tsconfig.json";
    }

    return "";
}

// কিছু Message ID ভিতরে আলাদা হলেও বাইরে থেকে একই রকম দেখায়. 
// ভবিষ্যতে প্রয়োজনে এগুলো আরও বাড়ানো যেতে পারে. 
// তাই বাইরে থেকে যেহেতু পার্থক্য বোঝা যায় না, 
// সেগুলোকে সুবিধার জন্য সবচেয়ে বড় Message ID-তে map করা হয়.

MsgID StringToMaximumMsgID(std::string_view id)
{
    std::unordered_map<MsgID, LogLevel> overrides;
    MsgID maxID = MsgID::kNone;

    StringToMsgIDs(id, LogLevel::kInfo, overrides);

    for (const auto& [msgID, level] : overrides) {
        if (msgID > maxID) {
            maxID = msgID;
        }
    }

    return maxID;
}

}