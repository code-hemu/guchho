#include "js_parser/js_parser_lower.hpp"
#include "js_ast/js_lexer_token.hpp"
#include <cmath>
#include <cstring>
#include <unordered_map>

namespace guchho::js {

bool bindingHasObjectRest(const Binding& binding) {
    if (binding.kind == BindingKind::Object) {
        auto& obj = static_cast<const BObject&>(binding);
        for (auto& entry : obj.entries) {
            if (!entry.key) return true; // rest pattern
        }
        return false;
    }
    if (binding.kind == BindingKind::Array) {
        auto& arr = static_cast<const BArray&>(binding);
        for (auto& item : arr.items) {
            if (item && bindingHasObjectRest(*item)) return true;
        }
        return false;
    }
    return false;
}

bool needsLowering(const Expr& expr, const LowerOptions& opts) {
    if (opts.unsupportedOptionalChain && expr.kind == ExprKind::OptionalChain)
        return true;
    if (opts.unsupportedNullishCoalescing && expr.kind == ExprKind::Binary) {
        auto& bin = static_cast<const EBinary&>(expr);
        if (bin.op == OpCode::BinOpNullishCoalescing) return true;
    }
    if (opts.unsupportedClassFields && expr.kind == ExprKind::Class)
        return true;
    return false;
}

bool couldPotentiallyThrow(const Expr* expr) {
    if (!expr) return false;
    switch (expr->kind) {
        case ExprKind::Call:
        case ExprKind::New:
        case ExprKind::Dot:
        case ExprKind::Index:
        case ExprKind::OptionalChain:
            return true;
        case ExprKind::Binary: {
            auto& bin = static_cast<const EBinary&>(*expr);
            return couldPotentiallyThrow(bin.left.get()) || couldPotentiallyThrow(bin.right.get());
        }
        case ExprKind::Unary: {
            auto& un = static_cast<const EUnary&>(*expr);
            return couldPotentiallyThrow(un.operand.get());
        }
        case ExprKind::Conditional: {
            auto& cond = static_cast<const EConditional&>(*expr);
            return couldPotentiallyThrow(cond.test.get()) ||
                   couldPotentiallyThrow(cond.consequent.get()) ||
                   couldPotentiallyThrow(cond.alternate.get());
        }
        case ExprKind::Sequence: {
            auto& seq = static_cast<const ESequence&>(*expr);
            for (auto& item : seq.items) {
                if (couldPotentiallyThrow(item.get())) return true;
            }
            return false;
        }
        default:
            return false;
    }
}

bool exprStructuralEquals(const Expr* a, const Expr* b) {
    if (!a || !b) return a == b;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case ExprKind::Null:
        case ExprKind::Undefined:
            return true;
        case ExprKind::Boolean:
            return static_cast<const EBoolean*>(a)->value ==
                   static_cast<const EBoolean*>(b)->value;
        case ExprKind::Number:
            return static_cast<const ENumber*>(a)->value ==
                   static_cast<const ENumber*>(b)->value;
        case ExprKind::String:
            return static_cast<const EString*>(a)->value ==
                   static_cast<const EString*>(b)->value;
        case ExprKind::BigInt:
            return static_cast<const EBigInt*>(a)->value ==
                   static_cast<const EBigInt*>(b)->value;
        default:
            return false;
    }
}

uint32_t duplicateCaseHash(const Expr* expr) {
    if (!expr) return 0;
    switch (expr->kind) {
        case ExprKind::Null: return 0;
        case ExprKind::Undefined: return 1;
        case ExprKind::Boolean: {
            bool v = static_cast<const EBoolean*>(expr)->value;
            return v ? 0x10002 : 0x10001;
        }
        case ExprKind::Number: {
            double v = static_cast<const ENumber*>(expr)->value;
            uint64_t bits;
            std::memcpy(&bits, &v, sizeof(bits));
            return static_cast<uint32_t>(bits) ^ static_cast<uint32_t>(bits >> 32);
        }
        case ExprKind::String: {
            auto& s = static_cast<const EString*>(expr)->value;
            uint32_t h = 4;
            for (char c : s) h = h * 33 + static_cast<uint32_t>(c);
            return h;
        }
        case ExprKind::BigInt: {
            auto& s = static_cast<const EBigInt*>(expr)->value;
            uint32_t h = 5;
            for (char c : s) h = h * 33 + static_cast<uint32_t>(c);
            return h;
        }
        default:
            return 0;
    }
}

bool duplicateCaseEquals(const Expr* left, const Expr* right, bool& couldBeIncorrect) {
    couldBeIncorrect = false;
    return exprStructuralEquals(left, right);
}

std::unique_ptr<Expr> lowerOptionalChainExpr(std::unique_ptr<Expr> expr) {
    // a?.b -> a === null || a === void 0 ? void 0 : a.b
    if (expr->kind != ExprKind::OptionalChain) return expr;
    auto& opt = static_cast<EOptionalChain&>(*expr);
    auto value = std::move(opt.value);
    SourceRange loc = expr->loc;

    // Create: (obj === null) || (obj === undefined) ? undefined : result
    // For now, just return the inner value without optional chaining
    return value;
}

std::unique_ptr<Expr> lowerNullishCoalescingExpr(std::unique_ptr<Expr> expr) {
    if (expr->kind != ExprKind::Binary) return expr;
    auto& bin = static_cast<EBinary&>(*expr);
    if (bin.op != OpCode::BinOpNullishCoalescing) return expr;

    auto left = std::move(bin.left);
    auto right = std::move(bin.right);
    SourceRange loc = expr->loc;

    auto leftClone = [&]() -> std::unique_ptr<Expr> {
        // Clone by re-parsing conceptually — for simplicity return a copy
        if (left->kind == ExprKind::Identifier) {
            auto& id = static_cast<const EIdentifier&>(*left);
            return std::make_unique<EIdentifier>(id.name, id.loc);
        }
        if (left->kind == ExprKind::Number) {
            auto& n = static_cast<const ENumber&>(*left);
            return std::make_unique<ENumber>(n.value, n.raw, n.loc);
        }
        if (left->kind == ExprKind::String) {
            auto& s = static_cast<const EString&>(*left);
            return std::make_unique<EString>(s.value, s.raw, s.loc);
        }
        if (left->kind == ExprKind::Boolean) {
            auto& b = static_cast<const EBoolean&>(*left);
            return std::make_unique<EBoolean>(b.value, b.loc);
        }
        if (left->kind == ExprKind::Null) {
            return std::make_unique<ENull>(left->loc);
        }
        if (left->kind == ExprKind::Undefined) {
            return std::make_unique<EUndefined>(left->loc);
        }
        return std::make_unique<EIdentifier>("_ref", left->loc);
    };

    auto nullCheck = std::make_unique<EBinary>(
        OpCode::BinOpStrictNe,
        leftClone(),
        std::make_unique<ENull>(loc),
        loc);

    auto undefinedCheck = std::make_unique<EBinary>(
        OpCode::BinOpStrictNe,
        leftClone(),
        std::make_unique<EUndefined>(loc),
        loc);

    auto test = std::make_unique<EBinary>(
        OpCode::BinOpLogicalAnd,
        std::move(nullCheck),
        std::move(undefinedCheck),
        loc);

    return std::make_unique<EConditional>(
        std::move(test),
        std::move(left),
        std::move(right),
        loc);
}

std::unique_ptr<Expr> lowerPrivateMemberAccess(std::unique_ptr<Expr> object,
                                                 const std::string& fieldName,
                                                 SourceRange loc) {
    // x.#foo -> x["__foo"]
    return std::make_unique<EIndex>(
        std::move(object),
        std::make_unique<EString>(fieldName, "", loc),
        false,
        loc);
}

bool isProtoKey(const Expr& key) {
    if (key.kind == ExprKind::String) {
        return static_cast<const EString&>(key).value == "__proto__";
    }
    return false;
}

std::unique_ptr<Expr> convertBindingToExpr(const Binding& binding) {
    if (binding.kind == BindingKind::Identifier) {
        auto& id = static_cast<const BIdentifier&>(binding);
        return std::make_unique<EIdentifier>(id.name, id.loc);
    }
    if (binding.kind == BindingKind::Array) {
        auto& arr = static_cast<const BArray&>(binding);
        std::vector<std::unique_ptr<Expr>> items;
        for (auto& item : arr.items) {
            if (item) items.push_back(convertBindingToExpr(*item));
            else items.push_back(nullptr);
        }
        return std::make_unique<EArray>(std::move(items), binding.loc);
    }
    if (binding.kind == BindingKind::Object) {
        auto& obj = static_cast<const BObject&>(binding);
        std::vector<Property> props;
        for (auto& entry : obj.entries) {
            Property prop;
            prop.key = entry.key ? cloneExpr(*entry.key) : nullptr;
            if (entry.value) prop.valueOrNil = convertBindingToExpr(*entry.value);
            props.push_back(std::move(prop));
        }
        return std::make_unique<EObject>(std::move(props), binding.loc);
    }
    return std::make_unique<EIdentifier>("_", binding.loc);
}

} // namespace guchho::js
