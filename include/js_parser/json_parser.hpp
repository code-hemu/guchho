#pragma once

#include "js_ast/js_ast_fwd.hpp"
#include "js_ast/js_ast_expr.hpp"
#include "js_ast/js_ast_stmt.hpp"
#include "js_lexer/js_lexer.hpp"
#include <string>
#include <memory>
#include <vector>

namespace guchho::js {

struct JSONOptions {
    bool allowComments = true;
    bool allowTrailingCommas = true;
    bool isForDefine = false;
};

// Parse a JSON/JSONC/JSON5 expression
std::unique_ptr<Expr> parseJSONExpr(JsLexer& lexer, const JSONOptions& opts,
                                     std::vector<ParseError>& errors);

// Top-level JSON parse function
std::unique_ptr<Expr> parseJSON(std::string_view source, const JSONOptions& opts,
                                 std::vector<ParseError>& errors);

// Check if an expression tree contains only valid JSON types
bool isValidJSON(const Expr& expr);

} // namespace guchho::js
