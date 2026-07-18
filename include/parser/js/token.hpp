#pragma once

#include <string_view>

namespace guchho::js {

enum class JsTokenType {
    Identifier,
    String,
    Number,
    Punctuation,
    Keyword,
    Operator,
    Comment,
    Whitespace,
    Unknown,
    EndOfFile,
};

struct JsSourceLocation {
    size_t offset = 0;
    size_t line = 1;
    size_t column = 1;
};

struct JsToken {
    JsTokenType type = JsTokenType::EndOfFile;
    std::string_view lexeme;
    JsSourceLocation start;
    JsSourceLocation end;
};

}
