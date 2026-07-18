#pragma once

#include "parser/css/token.hpp"
#include <string_view>
#include <vector>

namespace guchho::css {

class CssTokenizer {
public:
    explicit CssTokenizer(std::string_view source);

    CssToken peek();
    CssToken consume();
    bool has_next() const;
    std::vector<CssToken> tokenize();

private:
    std::string_view source_;
    size_t pos_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    char current() const;
    char peek_next(size_t ahead = 1) const;
    bool is_at_end() const;
    void advance();
    CssToken make_token(CssTokenType type, size_t start);
    void skip_whitespace();

    CssToken tokenize_ident();
    CssToken tokenize_number();
    CssToken tokenize_string();
    CssToken tokenize_at_keyword();
    CssToken tokenize_url();
};

}
