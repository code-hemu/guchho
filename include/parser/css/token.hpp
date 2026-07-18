#pragma once

#include <string_view>

namespace guchho::css {

enum class CssTokenType {
    Ident,
    AtKeyword,
    Hash,
    String,
    Number,
    Percentage,
    Dimension,
    Url,
    Function,
    Delim,
    Colon,
    Semicolon,
    Comma,
    LBrace,
    RBrace,
    LParen,
    RParen,
    Whitespace,
    Comment,
    EndOfFile,
};

struct CssSourceLocation {
    size_t offset = 0;
    size_t line = 1;
    size_t column = 1;
};

struct CssToken {
    CssTokenType type = CssTokenType::EndOfFile;
    std::string_view lexeme;
    CssSourceLocation start;
    CssSourceLocation end;
};

}
