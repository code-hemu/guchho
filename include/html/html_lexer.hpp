#pragma once

#include <string_view>
#include <vector>

namespace guchho::html {

enum class HtmlTokenType {
    TagOpen,
    TagClose,
    SelfClosing,
    AttributeName,
    AttributeValue,
    Text,
    Comment,
    Doctype,
    EndOfFile,
};

struct HtmlSourceLocation {
    size_t offset = 0;
    size_t line = 1;
    size_t column = 1;
};

struct HtmlToken {
    HtmlTokenType type = HtmlTokenType::EndOfFile;
    std::string_view lexeme;
    HtmlSourceLocation start;
    HtmlSourceLocation end;
};

class HtmlLexer {
public:
    explicit HtmlLexer(std::string_view source);

    HtmlToken peek();
    HtmlToken consume();
    bool has_next() const;
    std::vector<HtmlToken> tokenize();

private:
    std::string_view source_;
    size_t pos_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    char current() const;
    char peek_next(size_t ahead = 1) const;
    bool is_at_end() const;
    void advance();
    HtmlToken make_token(HtmlTokenType type, size_t start);
    void skip_whitespace();
};

}
