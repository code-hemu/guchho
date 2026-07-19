#include "js_parser/sourcemap_parser.hpp"
#include "js_parser/json_parser.hpp"
#include "js_ast/js_ast_expr.hpp"
#include "js_ast/js_ast_stmt.hpp"
#include "js_lexer/js_lexer.hpp"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdint>

namespace guchho::js {

// VLQ Base64 decoding
static const char* BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int8_t base64DecodeTable[128] = {-1};

static void initBase64Table() {
    static bool initialized = false;
    if (initialized) return;
    for (int i = 0; i < 128; i++) base64DecodeTable[i] = -1;
    for (int i = 0; BASE64_CHARS[i]; i++) {
        base64DecodeTable[static_cast<int>(BASE64_CHARS[i])] = static_cast<int8_t>(i);
    }
    initialized = true;
}

static int8_t decodeBase64(char c) {
    initBase64Table();
    if (c < 0 || c >= 128) return -1;
    return base64DecodeTable[static_cast<int>(c)];
}

VLQResult decodeVLQ(const std::string& input, size_t offset) {
    VLQResult result = {0, 0, false};
    if (offset >= input.size()) return result;

    int32_t value = 0;
    int32_t shift = 0;
    size_t pos = offset;

    while (pos < input.size()) {
        int8_t digit = decodeBase64(input[pos]);
        if (digit < 0) break;

        value |= static_cast<int32_t>(digit & 0x1F) << shift;
        shift += 5;
        pos++;

        if (!(digit & 0x20)) {
            // Last byte - handle sign
            if (value & 1) {
                value = -(value >> 1);
            } else {
                value = value >> 1;
            }
            result.value = value;
            result.length = static_cast<int32_t>(pos - offset);
            result.ok = true;
            return result;
        }
    }

    return result;
}

VLQResult decodeVLQFromUtf16(const std::vector<uint16_t>& input, size_t offset) {
    VLQResult result = {0, 0, false};
    if (offset >= input.size()) return result;

    int32_t value = 0;
    int32_t shift = 0;
    size_t pos = offset;

    while (pos < input.size()) {
        uint16_t c = input[pos];
        if (c > 127) break;
        int8_t digit = decodeBase64(static_cast<char>(c));
        if (digit < 0) break;

        value |= static_cast<int32_t>(digit & 0x1F) << shift;
        shift += 5;
        pos++;

        if (!(digit & 0x20)) {
            if (value & 1) {
                value = -(value >> 1);
            } else {
                value = value >> 1;
            }
            result.value = value;
            result.length = static_cast<int32_t>(pos - offset);
            result.ok = true;
            return result;
        }
    }

    return result;
}

static SourceMapData parseSourceMapFromExpr(std::unique_ptr<Expr> expr) {
    SourceMapData data;

    if (!expr || expr->kind != ExprKind::Object) return data;
    auto& obj = static_cast<EObject&>(*expr);
    if (obj.properties.empty()) return data;

    struct SourceMapSection {
        int32_t lineOffset = 0;
        int32_t columnOffset = 0;
        EObject* sourceMap = nullptr;
    };

    std::vector<SourceMapSection> sections;
    bool hasSections = false;

    for (auto& prop : obj.properties) {
        if (!prop.key || prop.key->kind != ExprKind::String) continue;
        auto& key = static_cast<EString&>(*prop.key);
        if (key.value == "sections" && prop.valueOrNil && prop.valueOrNil->kind == ExprKind::Array) {
            auto& arr = static_cast<EArray&>(*prop.valueOrNil);
            for (auto& item : arr.items) {
                if (!item || item->kind != ExprKind::Object) continue;
                auto& sectionObj = static_cast<EObject&>(*item);
                SourceMapSection section;
                for (auto& sectionProp : sectionObj.properties) {
                    if (!sectionProp.key || sectionProp.key->kind != ExprKind::String) continue;
                    auto& sectionKey = static_cast<EString&>(*sectionProp.key);
                    if (sectionKey.value == "offset" && sectionProp.valueOrNil &&
                        sectionProp.valueOrNil->kind == ExprKind::Object) {
                        auto& offsetObj = static_cast<EObject&>(*sectionProp.valueOrNil);
                        for (auto& offsetProp : offsetObj.properties) {
                            if (!offsetProp.key || offsetProp.key->kind != ExprKind::String) continue;
                            auto& offsetKey = static_cast<EString&>(*offsetProp.key);
                            if (offsetKey.value == "line" && offsetProp.valueOrNil &&
                                offsetProp.valueOrNil->kind == ExprKind::Number) {
                                section.lineOffset = static_cast<int32_t>(
                                    static_cast<ENumber&>(*offsetProp.valueOrNil).value);
                            } else if (offsetKey.value == "column" && offsetProp.valueOrNil &&
                                       offsetProp.valueOrNil->kind == ExprKind::Number) {
                                section.columnOffset = static_cast<int32_t>(
                                    static_cast<ENumber&>(*offsetProp.valueOrNil).value);
                            }
                        }
                    } else if (sectionKey.value == "map" && sectionProp.valueOrNil &&
                               sectionProp.valueOrNil->kind == ExprKind::Object) {
                        section.sourceMap = &static_cast<EObject&>(*sectionProp.valueOrNil);
                    }
                }
                if (section.sourceMap) {
                    sections.push_back(section);
                }
            }
            hasSections = true;
            break;
        }
    }

    if (!hasSections) {
        SourceMapSection section;
        section.sourceMap = &obj;
        sections.push_back(section);
    }

    std::vector<SourceMapEntry> mappings;
    int32_t generatedLine = 0;
    int32_t generatedColumn = 0;
    bool needSort = false;

    for (auto& section : sections) {
        if (!section.sourceMap) continue;

        std::vector<std::string> sourcesArray;
        std::vector<std::string> sourcesContentArray;
        std::vector<std::string> namesArray;
        std::string mappingsStr;
        std::string sourceRoot;
        bool hasVersion = false;

        for (auto& prop : section.sourceMap->properties) {
            if (!prop.key || prop.key->kind != ExprKind::String) continue;
            auto& key = static_cast<EString&>(*prop.key);

            if (key.value == "version" && prop.valueOrNil &&
                prop.valueOrNil->kind == ExprKind::Number) {
                auto& num = static_cast<ENumber&>(*prop.valueOrNil);
                if (num.value == 3.0) hasVersion = true;
            } else if (key.value == "mappings" && prop.valueOrNil &&
                       prop.valueOrNil->kind == ExprKind::String) {
                mappingsStr = static_cast<EString&>(*prop.valueOrNil).value;
            } else if (key.value == "sourceRoot" && prop.valueOrNil &&
                       prop.valueOrNil->kind == ExprKind::String) {
                sourceRoot = static_cast<EString&>(*prop.valueOrNil).value;
            } else if (key.value == "sources" && prop.valueOrNil &&
                       prop.valueOrNil->kind == ExprKind::Array) {
                auto& arr = static_cast<EArray&>(*prop.valueOrNil);
                for (auto& item : arr.items) {
                    if (item && item->kind == ExprKind::String) {
                        sourcesArray.push_back(static_cast<EString&>(*item).value);
                    } else {
                        sourcesArray.push_back("");
                    }
                }
            } else if (key.value == "sourcesContent" && prop.valueOrNil &&
                       prop.valueOrNil->kind == ExprKind::Array) {
                auto& arr = static_cast<EArray&>(*prop.valueOrNil);
                for (auto& item : arr.items) {
                    if (item && item->kind == ExprKind::String) {
                        sourcesContentArray.push_back(static_cast<EString&>(*item).value);
                    } else {
                        sourcesContentArray.push_back("");
                    }
                }
            } else if (key.value == "names" && prop.valueOrNil &&
                       prop.valueOrNil->kind == ExprKind::Array) {
                auto& arr = static_cast<EArray&>(*prop.valueOrNil);
                for (auto& item : arr.items) {
                    if (item && item->kind == ExprKind::String) {
                        namesArray.push_back(static_cast<EString&>(*item).value);
                    } else {
                        namesArray.push_back("");
                    }
                }
            }
        }

        if (!hasVersion || mappingsStr.empty() || sourcesArray.empty()) continue;

        int32_t lineOffset = section.lineOffset;
        int32_t columnOffset = section.columnOffset;
        int32_t sourceOffset = static_cast<int32_t>(data.sources.size());
        int32_t nameOffset = static_cast<int32_t>(data.names.size());

        if (lineOffset < generatedLine ||
            (lineOffset == generatedLine && columnOffset < generatedColumn)) {
            needSort = true;
        }

        generatedLine = lineOffset;
        generatedColumn = columnOffset;
        int32_t sourceIndex = sourceOffset;
        int32_t originalLine = 0;
        int32_t originalColumn = 0;
        int32_t originalName = nameOffset;

        // Parse VLQ mappings
        size_t current = 0;
        size_t mappingsLen = mappingsStr.size();

        while (current < mappingsLen) {
            if (mappingsStr[current] == ';') {
                generatedLine++;
                generatedColumn = 0;
                current++;
                continue;
            }

            // Read generated column
            auto colResult = decodeVLQ(mappingsStr, current);
            if (!colResult.ok) break;
            if (colResult.value < 0) needSort = true;
            generatedColumn += colResult.value;
            if (generatedColumn < 0) break;
            current += colResult.length;

            if (current >= mappingsLen) break;
            if (mappingsStr[current] == ',') { current++; continue; }
            if (mappingsStr[current] == ';') continue;

            // Read source index
            auto srcResult = decodeVLQ(mappingsStr, current);
            if (!srcResult.ok) break;
            sourceIndex += srcResult.value;
            current += srcResult.length;

            // Read original line
            auto lineResult = decodeVLQ(mappingsStr, current);
            if (!lineResult.ok) break;
            originalLine += lineResult.value;
            current += lineResult.length;

            // Read original column
            auto colResult2 = decodeVLQ(mappingsStr, current);
            if (!colResult2.ok) break;
            originalColumn += colResult2.value;
            current += colResult2.length;

            // Read name (optional)
            SourceMapEntry entry;
            entry.generatedLine = generatedLine;
            entry.generatedColumn = generatedColumn;
            entry.sourceIndex = sourceIndex;
            entry.originalLine = originalLine;
            entry.originalColumn = originalColumn;
            entry.originalNameIndex = -1;

            auto nameResult = decodeVLQ(mappingsStr, current);
            if (nameResult.ok) {
                originalName += nameResult.value;
                entry.originalNameIndex = originalName;
                current += nameResult.length;
            }

            mappings.push_back(entry);

            if (current < mappingsLen) {
                if (mappingsStr[current] == ',') current++;
            }
        }

        // Add sources/names from this section
        for (auto& s : sourcesArray) {
            data.sources.push_back(s);
        }
        for (auto& s : sourcesContentArray) {
            data.sourcesContent.push_back(s);
        }
        for (auto& n : namesArray) {
            data.names.push_back(n);
        }
    }

    if (needSort && !mappings.empty()) {
        std::stable_sort(mappings.begin(), mappings.end(),
            [](const SourceMapEntry& a, const SourceMapEntry& b) {
                return a.generatedLine < b.generatedLine ||
                       (a.generatedLine == b.generatedLine && a.generatedColumn <= b.generatedColumn);
            });
    }

    data.mappings = std::move(mappings);
    data.valid = !data.sources.empty() && !data.mappings.empty();
    return data;
}

SourceMapData parseSourceMap(std::string_view json) {
    SourceMapData data;

    std::vector<ParseError> errors;
    JSONOptions opts;
    opts.allowComments = true;
    opts.allowTrailingCommas = true;

    auto expr = parseJSON(json, opts, errors);
    if (!expr || !errors.empty()) return data;

    return parseSourceMapFromExpr(std::move(expr));
}

} // namespace guchho::js
