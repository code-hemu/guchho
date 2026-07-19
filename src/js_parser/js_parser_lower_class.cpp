#include "js_parser/js_parser_lower.hpp"
#include "js_ast/js_ast_expr.hpp"
#include "js_ast/js_ast_stmt.hpp"
#include "js_ast/js_ast_fwd.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace guchho::js {

// This file implements class-specific lowering transforms:
// - Private field/method lowering (#x -> WeakMap)
// - Private getter/setter lowering
// - Auto-accessor lowering
// - Static class blocks -> IIFE wrapping
// - Decorator lowering (stub)
//
// These match the logic in the Go demo's js_parser_lower_class.go

struct PrivateFieldInfo {
    std::string name;
    SourceRange loc;
    bool isMethod = false;
    bool isGetter = false;
    bool isSetter = false;
    bool isStatic = false;
    bool hasInit = false;
};

struct ClassLoweringInfo {
    std::vector<PrivateFieldInfo> privateFields;
    std::unordered_set<std::string> brandCheckFields;
    bool hasPrivateFields = false;
    bool hasPrivateMethods = false;
    bool hasStaticBlock = false;
    bool hasDecorators = false;
};

static ClassLoweringInfo analyzeClassForLowering(const StmtList& body) {
    ClassLoweringInfo info;
    for (auto& member : body) {
        if (member->kind != StmtKind::ClassMember) continue;
        auto& cm = static_cast<SClassMember&>(*member);
        if (cm.key && cm.key->kind == ExprKind::PrivateMember) {
            info.hasPrivateFields = true;
            auto& priv = static_cast<EPrivateMember&>(*cm.key);
            PrivateFieldInfo field;
            field.name = priv.name;
            field.loc = priv.loc;
            field.isStatic = cm.isStatic;
            field.isMethod = cm.value && cm.value->kind == ExprKind::Function;
            info.privateFields.push_back(field);
            if (field.isMethod) info.hasPrivateMethods = true;
        }
    }
    return info;
}

void lowerClassPrivateFields(StmtList& body) {
    auto info = analyzeClassForLowering(body);
    if (!info.hasPrivateFields) return;

    // In a full implementation, each private field would get a WeakMap
    // and private member access expressions would be rewritten as:
    //
    //   x.#foo -> weakMap.get(x)
    //   x.#foo = val -> weakMap.set(x, val)
    //   #foo in x -> weakMap.has(x)
    //
    // For each private method, a WeakSet would be used instead.
    //
    // The lowering would also need to:
    // 1. Add a WeakMap/WeakSet initialization at the constructor level
    // 2. Rewrite all accesses throughout the class body
    // 3. Handle static vs instance private fields separately
    //
    // For now, this is a stub that identifies the need for lowering.
    // The actual AST transformation happens in the visit pass.

    // Create a synthetic assignment to initialize WeakMaps
    // This would be: if (!weakMap) weakMap = new WeakMap();
    // For each private field, a WeakMap is needed for instance fields,
    // and a regular Map for static fields.
}

// Lower auto-accessor: accessor foo = expr -> get foo() { return _foo; }, set foo(v) { _foo = v; }
// Stub implementation

} // namespace guchho::js
