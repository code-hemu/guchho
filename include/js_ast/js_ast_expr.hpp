#pragma once

#include "js_ast_fwd.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace guchho::js {

struct Expr {
    ExprKind kind;
    SourceRange loc;
    Expr(ExprKind k, SourceRange l) : kind(k), loc(l) {}
    virtual ~Expr() = default;
    Expr(const Expr&) = delete;
    Expr& operator=(const Expr&) = delete;
    Expr(Expr&&) = default;
    Expr& operator=(Expr&&) = default;
};

struct EMissing : Expr {
    EMissing(SourceRange l) : Expr(ExprKind::Missing, l) {}
};

struct ENumber : Expr {
    double value;
    std::string raw;
    ENumber(double v, std::string r, SourceRange l)
        : Expr(ExprKind::Number, l), value(v), raw(std::move(r)) {}
};

struct EBigInt : Expr {
    std::string value;
    std::string raw;
    EBigInt(std::string v, std::string r, SourceRange l)
        : Expr(ExprKind::BigInt, l), value(std::move(v)), raw(std::move(r)) {}
};

struct EString : Expr {
    std::string value;
    std::string raw;
    EString(std::string v, std::string r, SourceRange l)
        : Expr(ExprKind::String, l), value(std::move(v)), raw(std::move(r)) {}
};

struct EBoolean : Expr {
    bool value;
    EBoolean(bool v, SourceRange l)
        : Expr(ExprKind::Boolean, l), value(v) {}
};

struct ENull : Expr {
    ENull(SourceRange l) : Expr(ExprKind::Null, l) {}
};

struct EUndefined : Expr {
    EUndefined(SourceRange l) : Expr(ExprKind::Undefined, l) {}
};

struct EThis : Expr {
    EThis(SourceRange l) : Expr(ExprKind::This, l) {}
};

struct ESuper : Expr {
    ESuper(SourceRange l) : Expr(ExprKind::Super, l) {}
};

struct ERegExp : Expr {
    std::string pattern;
    std::string flags;
    ERegExp(std::string p, std::string f, SourceRange l)
        : Expr(ExprKind::RegExp, l), pattern(std::move(p)), flags(std::move(f)) {}
};

struct EIdentifier : Expr {
    std::string name;
    bool mustKeepDueToWithStmt = false;
    bool canBeRemovedIfUnused = false;
    bool callCanBeUnwrappedIfUnused = false;
    EIdentifier(std::string n, SourceRange l)
        : Expr(ExprKind::Identifier, l), name(std::move(n)) {}
};

struct EImportIdentifier : Expr {
    std::string name;
    bool preferQuotedKey = false;
    bool wasOriginallyIdentifier = false;
    EImportIdentifier(std::string n, SourceRange l)
        : Expr(ExprKind::EImportIdentifier, l), name(std::move(n)) {}
};

struct EPrivateIdentifier : Expr {
    std::string name;
    EPrivateIdentifier(std::string n, SourceRange l)
        : Expr(ExprKind::EPrivateIdentifier, l), name(std::move(n)) {}
};

struct ENameOfSymbol : Expr {
    std::string name;
    bool hasPropertyKeyComment = false;
    ENameOfSymbol(std::string n, SourceRange l)
        : Expr(ExprKind::ENameOfSymbol, l), name(std::move(n)) {}
};

struct EBinary : Expr {
    OpCode op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    EBinary(OpCode o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, SourceRange loc)
        : Expr(ExprKind::Binary, loc), op(o), left(std::move(l)), right(std::move(r)) {}
};

struct EUnary : Expr {
    OpCode op;
    std::unique_ptr<Expr> operand;
    bool wasOriginallyTypeofIdentifier = false;
    bool wasOriginallyDeleteOfIdentifierOrPropertyAccess = false;
    EUnary(OpCode o, std::unique_ptr<Expr> opnd, SourceRange l)
        : Expr(ExprKind::Unary, l), op(o), operand(std::move(opnd)) {}
};

struct ESpan : Expr {
    OpCode op; // UnOpPreInc, UnOpPreDec, UnOpPostInc, UnOpPostDec
    std::unique_ptr<Expr> operand;
    ESpan(OpCode o, std::unique_ptr<Expr> opnd, SourceRange l)
        : Expr(ExprKind::Unary, l), op(o), operand(std::move(opnd)) {}
};

struct EConditional : Expr {
    std::unique_ptr<Expr> test;
    std::unique_ptr<Expr> consequent;
    std::unique_ptr<Expr> alternate;
    EConditional(std::unique_ptr<Expr> t, std::unique_ptr<Expr> c,
                 std::unique_ptr<Expr> a, SourceRange l)
        : Expr(ExprKind::Conditional, l),
          test(std::move(t)), consequent(std::move(c)), alternate(std::move(a)) {}
};

struct ESequence : Expr {
    std::vector<std::unique_ptr<Expr>> items;
    ESequence(std::vector<std::unique_ptr<Expr>> itms, SourceRange l)
        : Expr(ExprKind::Sequence, l), items(std::move(itms)) {}
};

struct ECall : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    std::optional<std::unique_ptr<Expr>> typeArgs;
    OptionalChain optionalChain = OptionalChain::None;
    CallKind callKind = CallKind::Normal;
    SourceLoc closeParenLoc;
    bool isMultiLine = false;
    bool canBeUnwrappedIfUnused = false;
    ECall(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> a, SourceRange l)
        : Expr(ExprKind::Call, l), callee(std::move(c)), args(std::move(a)) {}
};

struct ENew : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    SourceLoc closeParenLoc;
    bool isMultiLine = false;
    bool canBeUnwrappedIfUnused = false;
    ENew(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> a, SourceRange l)
        : Expr(ExprKind::New, l), callee(std::move(c)), args(std::move(a)) {}
};

struct EDot : Expr {
    std::unique_ptr<Expr> object;
    std::string property;
    bool optional = false;
    bool isPrivate = false;
    OptionalChain optionalChain = OptionalChain::None;
    bool canBeRemovedIfUnused = false;
    bool callCanBeUnwrappedIfUnused = false;
    bool isSymbolInstance = false;
    EDot(std::unique_ptr<Expr> obj, std::string prop, bool opt, SourceRange l)
        : Expr(ExprKind::Dot, l),
          object(std::move(obj)), property(std::move(prop)), optional(opt) {}
};

struct EIndex : Expr {
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;
    bool optional = false;
    OptionalChain optionalChain = OptionalChain::None;
    SourceLoc closeBracketLoc;
    bool canBeRemovedIfUnused = false;
    bool callCanBeUnwrappedIfUnused = false;
    bool isSymbolInstance = false;
    EIndex(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx, bool opt, SourceRange l)
        : Expr(ExprKind::Index, l),
          object(std::move(obj)), index(std::move(idx)), optional(opt) {}
};

struct EOptionalChain : Expr {
    std::unique_ptr<Expr> value;
    EOptionalChain(std::unique_ptr<Expr> v, SourceRange l)
        : Expr(ExprKind::OptionalChain, l), value(std::move(v)) {}
};

struct EArray : Expr {
    std::vector<std::unique_ptr<Expr>> items;
    SourceLoc closeBracketLoc;
    bool isSingleLine = false;
    bool isParenthesized = false;
    EArray(std::vector<std::unique_ptr<Expr>> itms, SourceRange l)
        : Expr(ExprKind::Array, l), items(std::move(itms)) {}
};

struct EObject : Expr {
    std::vector<Property> properties;
    SourceLoc closeBraceLoc;
    bool isSingleLine = false;
    bool isParenthesized = false;
    EObject(std::vector<Property> props, SourceRange l)
        : Expr(ExprKind::Object, l), properties(std::move(props)) {}
};

struct ESpread : Expr {
    std::unique_ptr<Expr> value;
    ESpread(std::unique_ptr<Expr> v, SourceRange l)
        : Expr(ExprKind::Spread, l), value(std::move(v)) {}
};

struct EAwait : Expr {
    std::unique_ptr<Expr> value;
    EAwait(std::unique_ptr<Expr> v, SourceRange l)
        : Expr(ExprKind::Await, l), value(std::move(v)) {}
};

struct EYield : Expr {
    std::unique_ptr<Expr> value;
    bool isDelegated = false;
    EYield(std::unique_ptr<Expr> v, bool del, SourceRange l)
        : Expr(ExprKind::Yield, l), value(std::move(v)), isDelegated(del) {}
};

struct ETemplatePart {
    std::string value;
    std::unique_ptr<Expr> expr;
};

struct ETemplate : Expr {
    std::vector<ETemplatePart> parts;
    std::string tail;
    bool canBeUnwrappedIfUnused = false;
    bool tagWasOriginallyPropertyAccess = false;
    ETemplate(std::vector<ETemplatePart> pts, std::string tl, SourceRange l)
        : Expr(ExprKind::Template, l), parts(std::move(pts)), tail(std::move(tl)) {}
};

struct EFunction : Expr {
    FnKind fnKind;
    std::string name;
    std::vector<std::unique_ptr<Binding>> params;
    StmtList body;
    std::unique_ptr<Expr> typeAnnotation;
    bool isParenthesized = false;
    EFunction(FnKind fk, std::string n,
              std::vector<std::unique_ptr<Binding>> p,
              StmtList b, SourceRange l)
        : Expr(ExprKind::Function, l),
          fnKind(fk), name(std::move(n)),
          params(std::move(p)), body(std::move(b)) {}
};

struct EArrow : Expr {
    FnKind fnKind;
    std::vector<std::unique_ptr<Binding>> params;
    std::unique_ptr<Expr> body;
    StmtList blockBody;
    bool hasBlockBody = false;
    bool preferExpr = false;
    bool isParenthesized = false;
    EArrow(FnKind fk,
           std::vector<std::unique_ptr<Binding>> p,
           std::unique_ptr<Expr> b, SourceRange l)
        : Expr(ExprKind::Arrow, l),
          fnKind(fk), params(std::move(p)), body(std::move(b)) {}
};

struct EClass : Expr {
    std::string name;
    std::unique_ptr<Expr> extends;
    StmtList body;
    EClass(std::string n, std::unique_ptr<Expr> e, StmtList b, SourceRange l)
        : Expr(ExprKind::Class, l),
          name(std::move(n)), extends(std::move(e)), body(std::move(b)) {}
};

struct EImport : Expr {
    std::string path;
    EImport(std::string p, SourceRange l)
        : Expr(ExprKind::Import, l), path(std::move(p)) {}
};

struct EImportCall : Expr {
    std::unique_ptr<Expr> expr;
    std::unique_ptr<Expr> options;
    SourceLoc closeParenLoc;
    EImportCall(std::unique_ptr<Expr> e, std::unique_ptr<Expr> o, SourceRange l)
        : Expr(ExprKind::ImportCall, l), expr(std::move(e)), options(std::move(o)) {}
};

struct ERequireCall : Expr {
    std::unique_ptr<Expr> expr;
    ERequireCall(std::unique_ptr<Expr> e, SourceRange l)
        : Expr(ExprKind::ERequireCall, l), expr(std::move(e)) {}
};

struct ERequireString : Expr {
    uint32_t importRecordIndex = 0;
    SourceLoc closeParenLoc;
    ERequireString(SourceRange l)
        : Expr(ExprKind::ERequireString, l) {}
};

struct ERequireResolveString : Expr {
    uint32_t importRecordIndex = 0;
    SourceLoc closeParenLoc;
    ERequireResolveString(SourceRange l)
        : Expr(ExprKind::ERequireResolveString, l) {}
};

struct EImportString : Expr {
    uint32_t importRecordIndex = 0;
    SourceLoc closeParenLoc;
    EImportString(SourceRange l)
        : Expr(ExprKind::EImportString, l) {}
};

struct EImportMeta : Expr {
    EImportMeta(SourceRange l) : Expr(ExprKind::EImportMeta, l) {}
};

struct ENewTarget : Expr {
    ENewTarget(SourceRange l) : Expr(ExprKind::ENewTarget, l) {}
};

struct EAnnotation : Expr {
    std::unique_ptr<Expr> value;
    AnnotationFlags flags = AnnotationFlags::None;
    EAnnotation(std::unique_ptr<Expr> v, AnnotationFlags f, SourceRange l)
        : Expr(ExprKind::EAnnotation, l), value(std::move(v)), flags(f) {}
};

struct EIf : Expr {
    std::unique_ptr<Expr> test;
    std::unique_ptr<Expr> yes;
    std::unique_ptr<Expr> no;
    EIf(std::unique_ptr<Expr> t, std::unique_ptr<Expr> y, std::unique_ptr<Expr> n, SourceRange l)
        : Expr(ExprKind::EIf, l), test(std::move(t)), yes(std::move(y)), no(std::move(n)) {}
};

struct EPrivateMember : Expr {
    std::unique_ptr<Expr> object;
    std::string name;
    bool isBrandCheck = false;
    EPrivateMember(std::unique_ptr<Expr> obj, std::string n, SourceRange l)
        : Expr(ExprKind::PrivateMember, l),
          object(std::move(obj)), name(std::move(n)) {}
};

struct EPrivateBrandCheck : Expr {
    std::unique_ptr<Expr> object;
    std::string field;
    EPrivateBrandCheck(std::unique_ptr<Expr> obj, std::string f, SourceRange l)
        : Expr(ExprKind::PrivateBrandCheck, l),
          object(std::move(obj)), field(std::move(f)) {}
};

struct EExportDefault : Expr {
    std::unique_ptr<Expr> value;
    EExportDefault(std::unique_ptr<Expr> v, SourceRange l)
        : Expr(ExprKind::ExportDefault, l), value(std::move(v)) {}
};

struct EJSXElement : Expr {
    std::string tagName;
    struct JSXAttr {
        std::string name;
        std::unique_ptr<Expr> value;
    };
    std::vector<JSXAttr> attributes;
    std::vector<std::unique_ptr<Expr>> children;
    bool selfClosing = false;
    EJSXElement(std::string tag, std::vector<JSXAttr> attrs,
                std::vector<std::unique_ptr<Expr>> ch, bool sc, SourceRange l)
        : Expr(ExprKind::JSXElement, l),
          tagName(std::move(tag)), attributes(std::move(attrs)),
          children(std::move(ch)), selfClosing(sc) {}
};

struct EJSXFragment : Expr {
    std::vector<std::unique_ptr<Expr>> children;
    EJSXFragment(std::vector<std::unique_ptr<Expr>> ch, SourceRange l)
        : Expr(ExprKind::JSXFragment, l), children(std::move(ch)) {}
};

struct EJSXText : Expr {
    std::string raw;
    EJSXText(std::string r, SourceRange l)
        : Expr(ExprKind::EJSXText, l), raw(std::move(r)) {}
};

} // namespace guchho::js
