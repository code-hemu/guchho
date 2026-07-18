#include "js_lexer/js_lexer.hpp"
#include <cctype>
#include <string>

namespace guchho::js {

JsLexer::JsLexer(std::string_view source)
    : source_(source) {}

char JsLexer::current() const {
    return source_[pos_];
}

char JsLexer::peek_next(size_t ahead) const {
    size_t idx = pos_ + ahead;
    return idx < source_.size() ? source_[idx] : '\0';
}

bool JsLexer::is_at_end() const {
    return pos_ >= source_.size();
}

void JsLexer::advance() {
    if (!is_at_end()) {
        if (current() == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
        ++pos_;
    }
}

JsToken JsLexer::make_token(JsTokenType type, size_t start) {
    JsToken t;
    t.type = type;
    t.lexeme = source_.substr(start, pos_ - start);
    t.start.offset = start;
    t.start.line = line_;
    t.start.column = column_;
    t.end.offset = pos_;
    t.end.line = line_;
    t.end.column = column_;
    return t;
}

void JsLexer::skip_whitespace() {
    while (!is_at_end() && (current() == ' ' || current() == '\t' ||
                            current() == '\n' || current() == '\r')) {
        advance();
    }
}

JsToken JsLexer::peek() {
    size_t saved_pos = pos_;
    size_t saved_line = line_;
    size_t saved_col = column_;
    JsToken t = consume();
    pos_ = saved_pos;
    line_ = saved_line;
    column_ = saved_col;
    return t;
}

JsToken JsLexer::consume() {
    skip_whitespace();

    if (is_at_end()) {
        JsToken t;
        t.type = JsTokenType::EndOfFile;
        t.start.offset = pos_;
        t.start.line = line_;
        t.start.column = column_;
        t.end = t.start;
        return t;
    }

    size_t start = pos_;
    char c = current();

    if (c == '"' || c == '\'' || c == '`') return tokenize_string();
    if (std::isdigit(c)) return tokenize_number();
    if (std::isalpha(c) || c == '_' || c == '$') return tokenize_identifier();

    if (c == '/' && peek_next() == '/') {
        advance(); advance();
        while (!is_at_end() && current() != '\n') advance();
        return make_token(JsTokenType::Comment, start);
    }

    if (c == '/' && peek_next() == '*') {
        advance(); advance();
        while (!is_at_end() && !(current() == '*' && peek_next() == '/')) advance();
        if (!is_at_end()) advance();
        if (!is_at_end()) advance();
        return make_token(JsTokenType::Comment, start);
    }

    return tokenize_operator_or_punctuation();
}

bool JsLexer::has_next() const {
    return pos_ < source_.size();
}

std::vector<JsToken> JsLexer::tokenize() {
    std::vector<JsToken> tokens;
    while (has_next()) {
        tokens.push_back(consume());
    }
    return tokens;
}

JsToken JsLexer::tokenize_identifier() {
    size_t start = pos_;
    while (!is_at_end() && (std::isalnum(current()) || current() == '_' ||
                            current() == '$')) {
        advance();
    }
    return make_token(JsTokenType::Identifier, start);
}

JsToken JsLexer::tokenize_number() {
    size_t start = pos_;
    while (!is_at_end() && (std::isdigit(current()) || current() == '.' ||
                            current() == 'x' || current() == 'X' ||
                            current() == 'e' || current() == 'E')) {
        advance();
    }
    return make_token(JsTokenType::Number, start);
}

JsToken JsLexer::tokenize_string() {
    size_t start = pos_;
    char quote = current();
    advance();
    while (!is_at_end() && current() != quote) {
        if (current() == '\\') advance();
        advance();
    }
    if (!is_at_end()) advance();
    return make_token(JsTokenType::String, start);
}

JsToken JsLexer::tokenize_operator_or_punctuation() {
    size_t start = pos_;
    advance();

    if (!is_at_end()) {
        std::string two{ source_[start], current() };
        if (two == "==" || two == "!=" || two == "<=" || two == ">=" ||
            two == "&&" || two == "||" || two == "++" || two == "--" ||
            two == "=>") {
            advance();
            return make_token(JsTokenType::Punctuation, start);
        }
    }

    return make_token(JsTokenType::Punctuation, start);
}

}
