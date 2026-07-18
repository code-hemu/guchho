#include "parser/html/tokenizer.hpp"

namespace guchho {

HtmlTokenizer::HtmlTokenizer(std::string_view source)
    : source_(source) {}

char HtmlTokenizer::current() const {
    return source_[pos_];
}

char HtmlTokenizer::peek_next(size_t ahead) const {
    size_t idx = pos_ + ahead;
    return idx < source_.size() ? source_[idx] : '\0';
}

bool HtmlTokenizer::is_at_end() const {
    return pos_ >= source_.size();
}

void HtmlTokenizer::advance() {
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

Token HtmlTokenizer::make_token(TokenType type, size_t start) {
    Token t;
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

void HtmlTokenizer::skip_whitespace() {
    while (!is_at_end() && (current() == ' ' || current() == '\t' ||
                            current() == '\n' || current() == '\r')) {
        advance();
    }
}

Token HtmlTokenizer::peek() {
    size_t saved_pos = pos_;
    size_t saved_line = line_;
    size_t saved_col = column_;
    Token t = consume();
    pos_ = saved_pos;
    line_ = saved_line;
    column_ = saved_col;
    return t;
}

Token HtmlTokenizer::consume() {
    skip_whitespace();

    if (is_at_end()) {
        Token t;
        t.type = TokenType::EOFToken;
        t.start.offset = pos_;
        t.start.line = line_;
        t.start.column = column_;
        t.end = t.start;
        return t;
    }

    size_t start = pos_;

    if (current() == '<') {
        if (peek_next() == '!') {
            if (peek_next(2) == '-' && peek_next(3) == '-') {
                return tokenize_comment();
            }
            if (peek_next(2) == 'D' || peek_next(2) == 'd') {
                return tokenize_doctype();
            }
        }
        if (peek_next() == '/') {
            return tokenize_tag(); // end tag
        }
        return tokenize_tag(); // start or self-closing tag
    }

    return tokenize_text();
}

bool HtmlTokenizer::has_next() const {
    return pos_ < source_.size();
}

std::vector<Token> HtmlTokenizer::tokenize() {
    std::vector<Token> tokens;
    while (has_next()) {
        tokens.push_back(consume());
    }
    return tokens;
}

Token HtmlTokenizer::tokenize_doctype() {
    size_t start = pos_;
    // Skip <!DOCTYPE
    while (!is_at_end() && current() != '>') {
        advance();
    }
    if (!is_at_end()) {
        advance(); // skip '>'
    }
    return make_token(TokenType::Doctype, start);
}

Token HtmlTokenizer::tokenize_comment() {
    size_t start = pos_;
    // Skip <!--
    advance(); advance(); advance(); advance(); // < ! - -
    while (!is_at_end()) {
        if (current() == '-' && peek_next() == '-' && peek_next(2) == '>') {
            advance(); advance(); advance(); // - - >
            break;
        }
        advance();
    }
    return make_token(TokenType::Comment, start);
}

Token HtmlTokenizer::tokenize_tag() {
    size_t start = pos_;
    bool is_end = false;

    if (current() == '<' && peek_next() == '/') {
        is_end = true;
        advance(); // <
        advance(); // /
    } else {
        advance(); // <
    }

    // Read tag name
    while (!is_at_end() && current() != '>' && current() != '/' &&
           current() != ' ' && current() != '\t' && current() != '\n') {
        advance();
    }

    TokenType type = is_end ? TokenType::EndTag : TokenType::StartTag;

    // Check for self-closing
    if (!is_at_end() && current() == '/') {
        advance(); // /
        type = TokenType::SelfClosingTag;
    }

    if (!is_at_end() && current() == '>') {
        advance();
    }

    return make_token(type, start);
}

Token HtmlTokenizer::tokenize_text() {
    size_t start = pos_;
    while (!is_at_end() && current() != '<') {
        advance();
    }
    return make_token(TokenType::Text, start);
}

}
