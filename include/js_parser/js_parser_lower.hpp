#pragma once

#include "js_ast/js_ast_fwd.hpp"
#include "js_ast/js_ast_expr.hpp"
#include "js_ast/js_ast_stmt.hpp"
#include <string>
#include <memory>

namespace guchho::js {

// Lowering functions for the JsParser visit pass
// These transform ES2020+ syntax into older JavaScript equivalents

struct LowerOptions {
    bool unsupportedOptionalChain = false;
    bool unsupportedNullishCoalescing = false;
    bool unsupportedAsyncAwait = false;
    bool unsupportedGenerator = false;
    bool unsupportedClassFields = false;
    bool unsupportedPrivateMembers = false;
    bool unsupportedObjectSpread = false;
    bool unsupportedArraySpread = false;
    bool unsupportedDestructuring = false;
    bool unsupportedForOf = false;
    bool unsupportedObjectAccessors = false;
    bool unsupportedConstAndLet = false;
    bool unsupportedExponentiation = false;
    bool unsupportedNumericSeparators = false;
};

// Check if a binding has an object rest pattern
bool bindingHasObjectRest(const Binding& binding);

// Check if we need to lower this expression
bool needsLowering(const Expr& expr, const LowerOptions& opts);

// Check if an expression could potentially throw
bool couldPotentiallyThrow(const Expr* expr);

// Check if two expressions are structurally equal
bool exprStructuralEquals(const Expr* a, const Expr* b);

// Hash a constant value for duplicate case detection
uint32_t duplicateCaseHash(const Expr* expr);

// Check if two expressions are equal for duplicate case checking
bool duplicateCaseEquals(const Expr* left, const Expr* right, bool& couldBeIncorrect);

// Lower optional chain: a?.b -> a == null ? void 0 : a.b
std::unique_ptr<Expr> lowerOptionalChainExpr(std::unique_ptr<Expr> expr);

// Lower nullish coalescing: a ?? b -> a !== null && a !== void 0 ? a : b
std::unique_ptr<Expr> lowerNullishCoalescingExpr(std::unique_ptr<Expr> expr);

// Lower private member access: x.#foo -> x["__foo"]
std::unique_ptr<Expr> lowerPrivateMemberAccess(std::unique_ptr<Expr> object,
                                                const std::string& fieldName,
                                                SourceRange loc);

// Lower class private fields to WeakMap usage
void lowerClassPrivateFields(StmtList& body);

// Determine if a property is a computed key "__proto__"
bool isProtoKey(const Expr& key);

// Convert binding to expression for destructuring lowering
std::unique_ptr<Expr> convertBindingToExpr(const Binding& binding);

// Deep-clone any expression
std::unique_ptr<Expr> cloneExpr(const Expr& expr);

} // namespace guchho::js
