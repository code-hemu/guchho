#include "js_parser/global_name_parser.hpp"
#include "js_ast/js_lexer_token.hpp"
#include "js_lexer/js_lexer.hpp"
#include <string>
#include <vector>

namespace guchho::js {

std::vector<std::string> parseGlobalName(std::string_view source) {
    std::vector<std::string> result;

    JsLexer lexer(source);

    // First token must be an identifier or keyword
    if (!lexer.peek().is(TokenType::Identifier) && !lexer.peek().isKeyword()) {
        return result; // empty = failure
    }

    // Handle special tokens
    if (lexer.peek().is(TokenType::This)) {
        result.push_back("this");
        lexer.next();
    } else if (lexer.peek().is(TokenType::Import)) {
        // import.meta
        lexer.next();
        if (lexer.peek().is(TokenType::Dot)) {
            lexer.next();
            if (lexer.peek().is(TokenType::Identifier) &&
                std::string(lexer.peek().lexeme) == "meta") {
                result.push_back("import");
                result.push_back("meta");
                lexer.next();
            } else {
                return result;
            }
        } else {
            return result;
        }
    } else {
        result.push_back(std::string(lexer.peek().lexeme));
        lexer.next();
    }

    // Follow with dot or index expressions
    while (!lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::Dot)) {
            lexer.next();
            if (!lexer.peek().is(TokenType::Identifier) && !lexer.peek().isKeyword()) {
                return result; // failure
            }
            result.push_back(std::string(lexer.peek().lexeme));
            lexer.next();
        } else if (lexer.peek().is(TokenType::OpenBracket)) {
            lexer.next();
            if (!lexer.peek().is(TokenType::StringLiteral)) {
                return result; // failure
            }
            std::string raw(lexer.peek().lexeme);
            std::string value;
            if (raw.size() >= 2) value = raw.substr(1, raw.size() - 2);
            result.push_back(value);
            lexer.next();
            if (!lexer.peek().is(TokenType::CloseBracket)) {
                return result; // failure
            }
            lexer.next();
        } else {
            break;
        }
    }

    return result;
}

} // namespace guchho::js
