#pragma once

#include <string_view>

namespace guchho {

enum class TokenType {
    Doctype,
    StartTag,
    EndTag,
    SelfClosingTag,
    AttributeName,
    AttributeValue,
    Text,
    Comment,
    EOFToken,
};

struct SourceLocation {
    size_t offset = 0;
    size_t line = 1;
    size_t column = 1;
};

struct Token {
    TokenType type = TokenType::EOFToken;
    std::string_view lexeme;
    SourceLocation start;
    SourceLocation end;
};

}
