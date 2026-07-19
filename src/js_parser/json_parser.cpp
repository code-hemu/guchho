#include "js_parser/json_parser.hpp"
#include "js_ast/js_lexer_token.hpp"
#include "js_lexer/js_lexer.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace guchho::js {

static bool skipJSONComma(JsLexer& lexer, TokenType closeToken, const JSONOptions& opts,
                           std::vector<ParseError>& errors) {
    if (!lexer.peek().is(TokenType::Comma)) {
        errors.push_back({"Expected ','", lexer.peek().range.start});
        return false;
    }
    lexer.next(); // consume comma

    if (lexer.peek().is(closeToken)) {
        if (!opts.allowTrailingCommas) {
            errors.push_back({"Trailing commas are not allowed in JSON", lexer.peek().range.start});
        }
        return false;
    }
    return true;
}

static std::unique_ptr<Expr> parseJSONValue(JsLexer& lexer, const JSONOptions& opts,
                                             std::vector<ParseError>& errors);

static std::unique_ptr<Expr> parseJSONObject(JsLexer& lexer, const JSONOptions& opts,
                                              std::vector<ParseError>& errors) {
    auto startLoc = lexer.peek().range.start;
    lexer.next(); // consume {

    std::vector<Property> properties;
    std::unordered_map<std::string, SourceRange> duplicateKeys;

    while (!lexer.peek().is(TokenType::CloseBrace) && !lexer.peek().is(TokenType::EndOfFile)) {
        if (!properties.empty()) {
            if (!skipJSONComma(lexer, TokenType::CloseBrace, opts, errors)) break;
        }

        // Key must be a string
        if (!lexer.peek().is(TokenType::StringLiteral)) {
            errors.push_back({"Expected string key in JSON object", lexer.peek().range.start});
            break;
        }

        auto keyRange = lexer.peek().range;
        std::string keyStr(lexer.peek().lexeme);
        // Strip quotes
        if (keyStr.size() >= 2) keyStr = keyStr.substr(1, keyStr.size() - 2);
        lexer.next();

        // Check for duplicate keys
        auto it = duplicateKeys.find(keyStr);
        if (it != duplicateKeys.end()) {
            errors.push_back({"Duplicate key '" + keyStr + "' in JSON object", keyRange.start});
        } else {
            duplicateKeys[keyStr] = keyRange;
        }

        // Colon
        if (!lexer.peek().is(TokenType::Colon)) {
            errors.push_back({"Expected ':' in JSON object", lexer.peek().range.start});
            break;
        }
        lexer.next();

        // Value
        Property prop;
        prop.key = std::make_unique<EString>(keyStr, std::string(lexer.peek().lexeme), keyRange);
        prop.valueOrNil = parseJSONValue(lexer, opts, errors);
        properties.push_back(std::move(prop));
    }

    lexer.expect(TokenType::CloseBrace);
    return std::make_unique<EObject>(std::move(properties),
        SourceRange{startLoc, lexer.peek().range.start});
}

static std::unique_ptr<Expr> parseJSONArray(JsLexer& lexer, const JSONOptions& opts,
                                             std::vector<ParseError>& errors) {
    auto startLoc = lexer.peek().range.start;
    lexer.next(); // consume [

    std::vector<std::unique_ptr<Expr>> items;

    while (!lexer.peek().is(TokenType::CloseBracket) && !lexer.peek().is(TokenType::EndOfFile)) {
        if (!items.empty()) {
            if (!skipJSONComma(lexer, TokenType::CloseBracket, opts, errors)) break;
        }
        items.push_back(parseJSONValue(lexer, opts, errors));
    }

    lexer.expect(TokenType::CloseBracket);
    return std::make_unique<EArray>(std::move(items),
        SourceRange{startLoc, lexer.peek().range.start});
}

static std::unique_ptr<Expr> parseJSONValue(JsLexer& lexer, const JSONOptions& opts,
                                             std::vector<ParseError>& errors) {
    auto& tok = lexer.peek();

    switch (tok.type) {
        case TokenType::StringLiteral: {
            std::string raw(tok.lexeme);
            std::string value;
            if (raw.size() >= 2) value = raw.substr(1, raw.size() - 2);
            lexer.next();
            return std::make_unique<EString>(value, raw, tok.range);
        }
        case TokenType::NumericLiteral: {
            double value = 0;
            try { value = std::stod(std::string(tok.lexeme)); } catch (...) {}
            lexer.next();
            return std::make_unique<ENumber>(value, std::string(tok.lexeme), tok.range);
        }
        case TokenType::Minus: {
            auto range = tok.range;
            lexer.next();
            if (!lexer.peek().is(TokenType::NumericLiteral)) {
                errors.push_back({"Expected number after '-' in JSON", lexer.peek().range.start});
                return std::make_unique<ENumber>(0, "", range);
            }
            double value = 0;
            try { value = std::stod(std::string(lexer.peek().lexeme)); } catch (...) {}
            lexer.next();
            return std::make_unique<ENumber>(-value, "", range);
        }
        case TokenType::True:
            lexer.next();
            return std::make_unique<EBoolean>(true, tok.range);
        case TokenType::False:
            lexer.next();
            return std::make_unique<EBoolean>(false, tok.range);
        case TokenType::Null:
            lexer.next();
            return std::make_unique<ENull>(tok.range);
        case TokenType::OpenBrace:
            return parseJSONObject(lexer, opts, errors);
        case TokenType::OpenBracket:
            return parseJSONArray(lexer, opts, errors);
        default: {
            std::string msg = "Unexpected token in JSON: ";
            msg += std::string(tokenTypeName(tok.type));
            errors.push_back({msg, tok.range.start});
            lexer.next();
            return std::make_unique<ENull>(tok.range);
        }
    }
}

std::unique_ptr<Expr> parseJSONExpr(JsLexer& lexer, const JSONOptions& opts,
                                     std::vector<ParseError>& errors) {
    auto result = parseJSONValue(lexer, opts, errors);
    if (!lexer.peek().is(TokenType::EndOfFile)) {
        errors.push_back({"Unexpected token after JSON value", lexer.peek().range.start});
    }
    return result;
}

std::unique_ptr<Expr> parseJSON(std::string_view source, const JSONOptions& opts,
                                 std::vector<ParseError>& errors) {
    JsLexer lexer(source);
    auto result = parseJSONValue(lexer, opts, errors);
    if (!lexer.peek().is(TokenType::EndOfFile)) {
        errors.push_back({"Unexpected content after JSON", lexer.peek().range.start});
    }
    return result;
}

bool isValidJSON(const Expr& expr) {
    switch (expr.kind) {
        case ExprKind::Null:
        case ExprKind::Boolean:
        case ExprKind::String:
        case ExprKind::Number:
            return true;
        case ExprKind::Array: {
            auto& arr = static_cast<const EArray&>(expr);
            for (auto& item : arr.items) {
                if (!item || !isValidJSON(*item)) return false;
            }
            return true;
        }
        case ExprKind::Object: {
            auto& obj = static_cast<const EObject&>(expr);
            for (auto& prop : obj.properties) {
                if (prop.isComputed()) return false;
                if (!prop.key || prop.key->kind != ExprKind::String) return false;
                if (!prop.valueOrNil || !isValidJSON(*prop.valueOrNil)) return false;
            }
            return true;
        }
        default:
            return false;
    }
}

} // namespace guchho::js
