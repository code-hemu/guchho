#pragma once

#include "parser/html/token.hpp"
#include <string_view>
#include <vector>

namespace guchho {

class HtmlTokenizer {
public:
    explicit HtmlTokenizer(std::string_view source);

    Token peek();
    Token consume();
    bool has_next() const;
    std::vector<Token> tokenize();

private:
    std::string_view source_;
    size_t pos_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    char current() const;
    char peek_next(size_t ahead = 1) const;
    bool is_at_end() const;
    void advance();
    Token make_token(TokenType type, size_t start);
    void skip_whitespace();

    Token tokenize_doctype();
    Token tokenize_tag();
    Token tokenize_comment();
    Token tokenize_text();
};

}
