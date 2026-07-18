#pragma once

#include "js_ast/js_token.hpp"
#include <string_view>
#include <vector>

namespace guchho::js {

class JsLexer {
public:
    explicit JsLexer(std::string_view source);

    JsToken peek();
    JsToken consume();
    bool has_next() const;
    std::vector<JsToken> tokenize();

private:
    std::string_view source_;
    size_t pos_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    char current() const;
    char peek_next(size_t ahead = 1) const;
    bool is_at_end() const;
    void advance();
    JsToken make_token(JsTokenType type, size_t start);
    void skip_whitespace();

    JsToken tokenize_identifier();
    JsToken tokenize_number();
    JsToken tokenize_string();
    JsToken tokenize_operator_or_punctuation();
};

}
