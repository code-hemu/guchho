#include "js_parser/ts_parser.hpp"
#include "js_ast/js_lexer_token.hpp"
#include <string>
#include <unordered_map>

namespace guchho::js {

// Map of TypeScript type identifier keywords
static const std::unordered_map<std::string, int> tsTypeIdentMap = {
    {"unique", 1}, {"abstract", 2}, {"asserts", 3},
    {"keyof", 4}, {"readonly", 4},
    {"any", 5}, {"never", 5}, {"unknown", 5}, {"undefined", 5},
    {"object", 5}, {"number", 5}, {"string", 5}, {"boolean", 5},
    {"bigint", 5}, {"symbol", 5},
    {"infer", 6},
};

void tsSkipBinding(JsLexer& lexer) {
    switch (lexer.peek().type) {
        case TokenType::Identifier:
        case TokenType::This:
            lexer.next();
            break;

        case TokenType::OpenBracket: {
            lexer.next();
            while (lexer.peek().is(TokenType::Comma)) lexer.next();
            while (!lexer.peek().is(TokenType::CloseBracket)) {
                if (lexer.peek().is(TokenType::DotDotDot)) lexer.next();
                tsSkipBinding(lexer);
                if (!lexer.peek().is(TokenType::Comma)) break;
                lexer.next();
            }
            lexer.expect(TokenType::CloseBracket);
            break;
        }

        case TokenType::OpenBrace: {
            lexer.next();
            while (!lexer.peek().is(TokenType::CloseBrace)) {
                bool foundIdent = false;
                switch (lexer.peek().type) {
                    case TokenType::DotDotDot:
                        lexer.next();
                        if (!lexer.peek().is(TokenType::Identifier)) lexer.expect(TokenType::Identifier);
                        foundIdent = true;
                        lexer.next();
                        break;
                    case TokenType::Identifier:
                        foundIdent = true;
                        lexer.next();
                        break;
                    case TokenType::StringLiteral:
                    case TokenType::NumericLiteral:
                        lexer.next();
                        break;
                    default:
                        if (lexer.peek().isKeyword()) lexer.next();
                        else lexer.expect(TokenType::Identifier);
                }
                if (lexer.peek().is(TokenType::Colon) || !foundIdent) {
                    lexer.expect(TokenType::Colon);
                    tsSkipBinding(lexer);
                }
                if (!lexer.peek().is(TokenType::Comma)) break;
                lexer.next();
            }
            lexer.expect(TokenType::CloseBrace);
            break;
        }

        default:
            lexer.expect(TokenType::Identifier);
    }
}

void tsSkipFnArgs(JsLexer& lexer) {
    lexer.expect(TokenType::OpenParen);
    while (!lexer.peek().is(TokenType::CloseParen)) {
        if (lexer.peek().is(TokenType::DotDotDot)) lexer.next();
        tsSkipBinding(lexer);
        if (lexer.peek().is(TokenType::Question)) lexer.next();
        if (lexer.peek().is(TokenType::Colon)) {
            lexer.next();
            tsSkipType(lexer, Precedence::Comma);
        }
        if (!lexer.peek().is(TokenType::Comma)) break;
        lexer.next();
    }
    lexer.expect(TokenType::CloseParen);
}

void tsSkipType(JsLexer& lexer, Precedence prec) {
    tsSkipTypeWithFlags(lexer, prec, 0);
}

void tsSkipTypeWithFlags(JsLexer& lexer, Precedence prec, uint8_t flags) {
    // Parse primary type
    switch (lexer.peek().type) {
        case TokenType::NumericLiteral:
        case TokenType::BigIntLiteral:
        case TokenType::StringLiteral:
        case TokenType::NoSubTemplateLiteral:
        case TokenType::True:
        case TokenType::False:
        case TokenType::Null:
        case TokenType::Void:
        case TokenType::Const:
        case TokenType::This:
            lexer.next();
            // Handle 'this is type' return type
            if (lexer.peek().is(TokenType::Identifier) && std::string(lexer.peek().lexeme) == "is" && !lexer.peek().hasNewlineBefore) {
                lexer.next();
                tsSkipType(lexer, Precedence::Comma);
            }
            break;

        case TokenType::Minus:
            lexer.next();
            if (lexer.peek().is(TokenType::BigIntLiteral)) lexer.next();
            else lexer.expect(TokenType::NumericLiteral);
            break;

        case TokenType::Ampersand:
        case TokenType::Bar:
            lexer.next();
            tsSkipTypeWithFlags(lexer, prec, flags);
            return;

        case TokenType::Import:
            lexer.next();
            if (flags & TS_ALLOW_TUPLE_LABELS) {
                if (lexer.peek().is(TokenType::Colon) || lexer.peek().is(TokenType::Question)) return;
            }
            lexer.expect(TokenType::OpenParen);
            lexer.expect(TokenType::StringLiteral);
            if (lexer.peek().is(TokenType::Comma)) {
                lexer.next();
                tsSkipObjectType(lexer);
                if (lexer.peek().is(TokenType::Comma)) lexer.next();
            }
            lexer.expect(TokenType::CloseParen);
            break;

        case TokenType::New:
            lexer.next();
            if (flags & TS_ALLOW_TUPLE_LABELS) {
                if (lexer.peek().is(TokenType::Colon) || lexer.peek().is(TokenType::Question)) return;
            }
            tsSkipTypeParameters(lexer);
            tsSkipParenOrFnType(lexer);
            break;

        case TokenType::LessThan:
            tsSkipTypeParameters(lexer);
            tsSkipParenOrFnType(lexer);
            break;

        case TokenType::OpenParen:
            tsSkipParenOrFnType(lexer);
            break;

        case TokenType::Identifier: {
            std::string idStr(lexer.peek().lexeme);
            auto it = tsTypeIdentMap.find(idStr);
            bool checkTypeParams = true;

            if (it != tsTypeIdentMap.end()) {
                lexer.next();
                switch (it->second) {
                    case 4: // keyof, readonly
                        if ((lexer.peek().is(TokenType::Colon) || lexer.peek().is(TokenType::Question) || lexer.peek().is(TokenType::In)) &&
                            !(flags & TS_IS_INDEX_SIGNATURE) && !(flags & TS_ALLOW_TUPLE_LABELS)) {
                            tsSkipTypeWithFlags(lexer, Precedence::Prefix, flags);
                        }
                        return;
                    case 6: // infer
                        if ((lexer.peek().is(TokenType::Colon) || lexer.peek().is(TokenType::Question) || lexer.peek().is(TokenType::In)) &&
                            !(flags & TS_IS_INDEX_SIGNATURE) && !(flags & TS_ALLOW_TUPLE_LABELS)) {
                            lexer.expect(TokenType::Identifier);
                        }
                        return;
                    case 5: // primitive types
                        checkTypeParams = false;
                        break;
                    default:
                        break;
                }
            } else {
                lexer.next();
            }

            // 'is' keyword for type predicates
            if (lexer.peek().is(TokenType::Identifier) && std::string(lexer.peek().lexeme) == "is" && !lexer.peek().hasNewlineBefore) {
                lexer.next();
                tsSkipType(lexer, Precedence::Comma);
                return;
            }

            if (checkTypeParams && !lexer.peek().hasNewlineBefore) {
                tsSkipTypeParameters(lexer);
            }
            break;
        }

        case TokenType::Typeof:
            lexer.next();
            if (flags & TS_ALLOW_TUPLE_LABELS) {
                if (lexer.peek().is(TokenType::Colon) || lexer.peek().is(TokenType::Question)) return;
            }
            if (lexer.peek().is(TokenType::Import)) {
                // typeof import('fs')
                break;
            }
            if (!(lexer.peek().is(TokenType::Identifier) || lexer.peek().isKeyword()))
                lexer.expect(TokenType::Identifier);
            lexer.next();
            while (lexer.peek().is(TokenType::Dot)) {
                lexer.next();
                if (!(lexer.peek().is(TokenType::Identifier) || lexer.peek().isKeyword()) &&
                    !(lexer.peek().lexeme.size() > 0 && lexer.peek().lexeme[0] == '#'))
                    lexer.expect(TokenType::Identifier);
                lexer.next();
            }
            if (!lexer.peek().hasNewlineBefore) {
                tsSkipTypeArguments(lexer);
            }
            break;

        case TokenType::OpenBracket: {
            // Tuple type: [number, string]
            lexer.next();
            while (!lexer.peek().is(TokenType::CloseBracket)) {
                if (lexer.peek().is(TokenType::DotDotDot)) lexer.next();
                tsSkipTypeWithFlags(lexer, Precedence::Comma, TS_ALLOW_TUPLE_LABELS);
                if (lexer.peek().is(TokenType::Question)) lexer.next();
                if (lexer.peek().is(TokenType::Colon)) {
                    lexer.next();
                    tsSkipType(lexer, Precedence::Comma);
                }
                if (!lexer.peek().is(TokenType::Comma)) break;
                lexer.next();
            }
            lexer.expect(TokenType::CloseBracket);
            break;
        }

        case TokenType::OpenBrace:
            tsSkipObjectType(lexer);
            break;

        case TokenType::TemplateHead:
            while (true) {
                lexer.next();
                tsSkipType(lexer, Precedence::Comma);
                // Rescan close brace as template token
                if (lexer.peek().is(TokenType::TemplateTail)) {
                    lexer.next();
                    break;
                }
            }
            break;

        default:
            lexer.expect(TokenType::Identifier);
    }

    // Handle union/intersection types
    while (true) {
        if (lexer.peek().is(TokenType::Bar)) {
            if (static_cast<int>(prec) >= static_cast<int>(Precedence::BitwiseOr)) return;
            lexer.next();
            tsSkipTypeWithFlags(lexer, Precedence::BitwiseOr, flags);
        } else if (lexer.peek().is(TokenType::Ampersand)) {
            if (static_cast<int>(prec) >= static_cast<int>(Precedence::BitwiseAnd)) return;
            lexer.next();
            tsSkipTypeWithFlags(lexer, Precedence::BitwiseAnd, flags);
        } else {
            break;
        }
    }

    // Handle extends ... ? ... : ... (conditional types)
    if (lexer.peek().is(TokenType::Extends) && !(flags & TS_DISALLOW_CONDITIONAL_TYPES)) {
        if (static_cast<int>(prec) >= static_cast<int>(Precedence::Conditional)) return;
        lexer.next();
        tsSkipTypeWithFlags(lexer, Precedence::BitwiseAnd, flags);
        lexer.expect(TokenType::Question);
        tsSkipTypeWithFlags(lexer, Precedence::Conditional, flags);
        lexer.expect(TokenType::Colon);
        tsSkipType(lexer, Precedence::Conditional);
    }
}

void tsSkipObjectType(JsLexer& lexer) {
    lexer.expect(TokenType::OpenBrace);
    int depth = 1;
    while (depth > 0 && !lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::OpenBrace)) { depth++; lexer.next(); }
        else if (lexer.peek().is(TokenType::CloseBrace)) { depth--; if (depth > 0) lexer.next(); }
        else if (lexer.peek().is(TokenType::Semicolon)) { lexer.next(); }
        else { lexer.next(); }
    }
    lexer.expect(TokenType::CloseBrace);
}

void tsSkipParenOrFnType(JsLexer& lexer) {
    // Try to skip as arrow function args first, otherwise as parenthesized type
    lexer.expect(TokenType::OpenParen);
    int depth = 1;
    while (depth > 0 && !lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::OpenParen)) { depth++; lexer.next(); }
        else if (lexer.peek().is(TokenType::CloseParen)) { depth--; if (depth > 0) lexer.next(); else break; }
        else { lexer.next(); }
    }
    lexer.expect(TokenType::CloseParen);

    // Check for => (arrow function type)
    if (lexer.peek().is(TokenType::Arrow)) {
        lexer.next();
        tsSkipReturnType(lexer);
    }
}

void tsSkipReturnType(JsLexer& lexer) {
    if (lexer.peek().is(TokenType::Colon)) lexer.next();
    tsSkipTypeWithFlags(lexer, Precedence::Comma, TS_IS_RETURN_TYPE);
}

void tsSkipTypeParameters(JsLexer& lexer) {
    if (!lexer.peek().is(TokenType::LessThan)) return;
    lexer.next();
    int depth = 1;
    while (depth > 0 && !lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::LessThan)) { depth++; lexer.next(); }
        else if (lexer.peek().is(TokenType::GreaterThan) ||
                 lexer.peek().is(TokenType::GreaterThanGreaterThan) ||
                 lexer.peek().is(TokenType::GreaterThanGreaterThanGreaterThan)) {
            depth--;
            lexer.next();
        } else {
            lexer.next();
        }
    }
}

void tsSkipTypeArguments(JsLexer& lexer) {
    tsSkipTypeParameters(lexer); // Same logic
}

void tsSkipEnumBody(JsLexer& lexer) {
    if (!lexer.peek().is(TokenType::OpenBrace)) return;
    lexer.next();
    int depth = 1;
    while (depth > 0 && !lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::OpenBrace)) { depth++; lexer.next(); }
        else if (lexer.peek().is(TokenType::CloseBrace)) { depth--; lexer.next(); }
        else { lexer.next(); }
    }
}

void tsSkipInterfaceBody(JsLexer& lexer) {
    if (!lexer.peek().is(TokenType::OpenBrace)) return;
    lexer.next();
    int depth = 1;
    while (depth > 0 && !lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::OpenBrace)) { depth++; lexer.next(); }
        else if (lexer.peek().is(TokenType::CloseBrace)) { depth--; lexer.next(); }
        else { lexer.next(); }
    }
}

void tsSkipNamespaceBody(JsLexer& lexer) {
    if (!lexer.peek().is(TokenType::OpenBrace)) return;
    lexer.next();
    int depth = 1;
    while (depth > 0 && !lexer.peek().is(TokenType::EndOfFile)) {
        if (lexer.peek().is(TokenType::OpenBrace)) { depth++; lexer.next(); }
        else if (lexer.peek().is(TokenType::CloseBrace)) { depth--; lexer.next(); }
        else { lexer.next(); }
    }
}

} // namespace guchho::js
