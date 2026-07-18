#include "html/html_lexer.hpp"
#include <cctype>

namespace guchho::html {

HtmlLexer::HtmlLexer(std::string_view source)
    : source_(source) {}

char HtmlLexer::current() const {
    return source_[pos_];
}

char HtmlLexer::peek_next(size_t ahead) const {
    size_t idx = pos_ + ahead;
    return idx < source_.size() ? source_[idx] : '\0';
}

bool HtmlLexer::is_at_end() const {
    return pos_ >= source_.size();
}

void HtmlLexer::advance() {
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

HtmlToken HtmlLexer::make_token(HtmlTokenType type, size_t start) {
    HtmlToken t;
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

void HtmlLexer::skip_whitespace() {
    while (!is_at_end() && (current() == ' ' || current() == '\t' ||
                            current() == '\n' || current() == '\r')) {
        advance();
    }
}

HtmlToken HtmlLexer::peek() {
    size_t saved_pos = pos_;
    size_t saved_line = line_;
    size_t saved_col = column_;
    HtmlToken t = consume();
    pos_ = saved_pos;
    line_ = saved_line;
    column_ = saved_col;
    return t;
}

HtmlToken HtmlLexer::consume() {
    skip_whitespace();

    if (is_at_end()) {
        return make_token(HtmlTokenType::EndOfFile, pos_);
    }

    size_t start = pos_;
    char c = current();

    if (c == '<') {
        advance();
        if (current() == '/') { advance(); return make_token(HtmlTokenType::TagClose, start); }
        if (current() == '!') {
            advance();
            if (peek_next() == '-' && peek_next(2) == '-') {
                advance(); advance();
                while (!is_at_end() && !(current() == '-' && peek_next() == '-' && peek_next(2) == '>')) advance();
                if (!is_at_end()) advance();
                if (!is_at_end()) advance();
                if (!is_at_end()) advance();
                return make_token(HtmlTokenType::Comment, start);
            }
            return make_token(HtmlTokenType::Doctype, start);
        }
        return make_token(HtmlTokenType::TagOpen, start);
    }

    while (!is_at_end() && current() != '<') advance();
    return make_token(HtmlTokenType::Text, start);
}

bool HtmlLexer::has_next() const {
    return pos_ < source_.size();
}

std::vector<HtmlToken> HtmlLexer::tokenize() {
    std::vector<HtmlToken> tokens;
    while (has_next()) {
        tokens.push_back(consume());
    }
    return tokens;
}

}
