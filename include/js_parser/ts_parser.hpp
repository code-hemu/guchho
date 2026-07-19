#pragma once

#include "js_ast/js_ast_fwd.hpp"
#include "js_ast/js_ast_expr.hpp"
#include "js_ast/js_ast_stmt.hpp"
#include "js_lexer/js_lexer.hpp"
#include "js_parser/js_parser.hpp"
#include <string>
#include <memory>

namespace guchho::js {

// TypeScript-specific flags for type skipping
const uint8_t TS_IS_RETURN_TYPE = 1;
const uint8_t TS_IS_INDEX_SIGNATURE = 2;
const uint8_t TS_ALLOW_TUPLE_LABELS = 4;
const uint8_t TS_DISALLOW_CONDITIONAL_TYPES = 8;

// Skip over a TypeScript binding pattern (destructuring)
void tsSkipBinding(JsLexer& lexer);

// Skip over TypeScript function arguments
void tsSkipFnArgs(JsLexer& lexer);

// Skip over a TypeScript type expression
void tsSkipType(JsLexer& lexer, Precedence prec);

// Skip over a TypeScript type expression with flags
void tsSkipTypeWithFlags(JsLexer& lexer, Precedence prec, uint8_t flags);

// Skip over a TypeScript object type expression
void tsSkipObjectType(JsLexer& lexer);

// Skip over a TypeScript paren or function type expression
void tsSkipParenOrFnType(JsLexer& lexer);

// Skip over a TypeScript return type
void tsSkipReturnType(JsLexer& lexer);

// Skip over TypeScript type parameters (<T, U>)
void tsSkipTypeParameters(JsLexer& lexer);

// Skip over TypeScript type arguments (<number, string>)
void tsSkipTypeArguments(JsLexer& lexer);

// Skip over a TypeScript enum body
void tsSkipEnumBody(JsLexer& lexer);

// Skip over a TypeScript interface body
void tsSkipInterfaceBody(JsLexer& lexer);

// Skip over a TypeScript namespace body
void tsSkipNamespaceBody(JsLexer& lexer);

} // namespace guchho::js
