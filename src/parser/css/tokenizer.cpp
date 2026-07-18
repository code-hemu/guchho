#include "parser/css/tokenizer.hpp"
#include <cctype>

namespace guchho::css {

CssTokenizer::CssTokenizer(std::string_view source)
    : source_(source) {}

char CssTokenizer::current() const {
    return source_[pos_];
}

char CssTokenizer::peek_next(size_t ahead) const {
    size_t idx = pos_ + ahead;
    return idx < source_.size() ? source_[idx] : '\0';
}

bool CssTokenizer::is_at_end() const {
    return pos_ >= source_.size();
}

void CssTokenizer::advance() {
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

CssToken CssTokenizer::make_token(CssTokenType type, size_t start) {
    CssToken t;
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

void CssTokenizer::skip_whitespace() {
    while (!is_at_end() && (current() == ' ' || current() == '\t' ||
                            current() == '\n' || current() == '\r')) {
        advance();
    }
}

CssToken CssTokenizer::peek() {
    size_t saved_pos = pos_;
    size_t saved_line = line_;
    size_t saved_col = column_;
    CssToken t = consume();
    pos_ = saved_pos;
    line_ = saved_line;
    column_ = saved_col;
    return t;
}

CssToken CssTokenizer::consume() {
    skip_whitespace();

    if (is_at_end()) {
        CssToken t;
        t.type = CssTokenType::EndOfFile;
        t.start.offset = pos_;
        t.start.line = line_;
        t.start.column = column_;
        t.end = t.start;
        return t;
    }

    size_t start = pos_;
    char c = current();

    if (c == '@') return tokenize_at_keyword();
    if (c == '"' || c == '\'') return tokenize_string();
    if (std::isdigit(c)) return tokenize_number();
    if (std::isalpha(c) || c == '_' || c == '-') return tokenize_ident();
    if (c == 'u' || c == 'U') {
        if (peek_next() == 'r' || peek_next() == 'R') {
            return tokenize_url();
        }
        return tokenize_ident();
    }
    if (c == '{') { advance(); return make_token(CssTokenType::LBrace, start); }
    if (c == '}') { advance(); return make_token(CssTokenType::RBrace, start); }
    if (c == '(') { advance(); return make_token(CssTokenType::LParen, start); }
    if (c == ')') { advance(); return make_token(CssTokenType::RParen, start); }
    if (c == ':') { advance(); return make_token(CssTokenType::Colon, start); }
    if (c == ';') { advance(); return make_token(CssTokenType::Semicolon, start); }
    if (c == ',') { advance(); return make_token(CssTokenType::Comma, start); }

    advance();
    return make_token(CssTokenType::Delim, start);
}

bool CssTokenizer::has_next() const {
    return pos_ < source_.size();
}

std::vector<CssToken> CssTokenizer::tokenize() {
    std::vector<CssToken> tokens;
    while (has_next()) {
        tokens.push_back(consume());
    }
    return tokens;
}

CssToken CssTokenizer::tokenize_ident() {
    size_t start = pos_;
    while (!is_at_end() && (std::isalnum(current()) || current() == '_' ||
                            current() == '-')) {
        advance();
    }
    return make_token(CssTokenType::Ident, start);
}

CssToken CssTokenizer::tokenize_number() {
    size_t start = pos_;
    while (!is_at_end() && (std::isdigit(current()) || current() == '.')) {
        advance();
    }
    return make_token(CssTokenType::Number, start);
}

CssToken CssTokenizer::tokenize_string() {
    size_t start = pos_;
    char quote = current();
    advance(); // skip opening quote
    while (!is_at_end() && current() != quote) {
        if (current() == '\\') advance();
        advance();
    }
    if (!is_at_end()) advance(); // skip closing quote
    return make_token(CssTokenType::String, start);
}

CssToken CssTokenizer::tokenize_at_keyword() {
    size_t start = pos_;
    advance(); // skip @
    while (!is_at_end() && (std::isalnum(current()) || current() == '_' ||
                            current() == '-')) {
        advance();
    }
    return make_token(CssTokenType::AtKeyword, start);
}

CssToken CssTokenizer::tokenize_url() {
    size_t start = pos_;
    advance(); advance(); // u, r
    if (!is_at_end() && current() == 'l') advance();
    if (!is_at_end() && current() == '(') {
        advance(); // (
        while (!is_at_end() && current() != ')') advance();
        if (!is_at_end()) advance(); // )
    }
    return make_token(CssTokenType::Url, start);
}

}
