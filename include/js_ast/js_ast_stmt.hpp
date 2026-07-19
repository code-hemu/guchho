#pragma once

#include "js_ast_fwd.hpp"
#include "js_ast_expr.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace guchho::js {

struct Stmt {
    StmtKind kind;
    SourceRange loc;
    Stmt(StmtKind k, SourceRange l) : kind(k), loc(l) {}
    virtual ~Stmt() = default;
    Stmt(const Stmt&) = delete;
    Stmt& operator=(const Stmt&) = delete;
    Stmt(Stmt&&) = default;
    Stmt& operator=(Stmt&&) = default;
};

struct SBlock : Stmt {
    StmtList stmts;
    SourceLoc closeBraceLoc;
    SBlock(StmtList s, SourceRange l)
        : Stmt(StmtKind::Block, l), stmts(std::move(s)) {}
};

struct SEmpty : Stmt {
    SEmpty(SourceRange l) : Stmt(StmtKind::Empty, l) {}
};

struct SComment : Stmt {
    std::string text;
    bool isLegalComment = false;
    SComment(std::string t, SourceRange l)
        : Stmt(StmtKind::SComment, l), text(std::move(t)) {}
};

struct SDirective : Stmt {
    std::string value;
    SourceLoc legacyOctalLoc;
    SDirective(std::string v, SourceRange l)
        : Stmt(StmtKind::SDirective, l), value(std::move(v)) {}
};

struct STypeScript : Stmt {
    bool wasDeclareClass = false;
    STypeScript(SourceRange l)
        : Stmt(StmtKind::STypeScript, l) {}
};

struct SExpression : Stmt {
    std::unique_ptr<Expr> expr;
    bool isFromClassOrFnThatCanBeRemovedIfUnused = false;
    SExpression(std::unique_ptr<Expr> e, SourceRange l)
        : Stmt(StmtKind::Expression, l), expr(std::move(e)) {}
};

struct SIf : Stmt {
    std::unique_ptr<Expr> test;
    std::unique_ptr<Stmt> consequent;
    std::unique_ptr<Stmt> alternate;
    bool isSingleLineYes = false;
    bool isSingleLineNo = false;
    SIf(std::unique_ptr<Expr> t, std::unique_ptr<Stmt> c,
        std::unique_ptr<Stmt> a, SourceRange l)
        : Stmt(StmtKind::If, l),
          test(std::move(t)), consequent(std::move(c)), alternate(std::move(a)) {}
};

struct SWhile : Stmt {
    std::unique_ptr<Expr> test;
    std::unique_ptr<Stmt> body;
    bool isSingleLineBody = false;
    SWhile(std::unique_ptr<Expr> t, std::unique_ptr<Stmt> b, SourceRange l)
        : Stmt(StmtKind::While, l), test(std::move(t)), body(std::move(b)) {}
};

struct SDoWhile : Stmt {
    std::unique_ptr<Expr> test;
    std::unique_ptr<Stmt> body;
    SDoWhile(std::unique_ptr<Expr> t, std::unique_ptr<Stmt> b, SourceRange l)
        : Stmt(StmtKind::DoWhile, l), test(std::move(t)), body(std::move(b)) {}
};

struct SFor : Stmt {
    std::unique_ptr<Expr> init;
    std::unique_ptr<Expr> test;
    std::unique_ptr<Expr> update;
    std::unique_ptr<Stmt> body;
    bool isSingleLineBody = false;
    bool isLoweredForAwait = false;
    SFor(std::unique_ptr<Expr> i, std::unique_ptr<Expr> t,
         std::unique_ptr<Expr> u, std::unique_ptr<Stmt> b, SourceRange l)
        : Stmt(StmtKind::For, l),
          init(std::move(i)), test(std::move(t)),
          update(std::move(u)), body(std::move(b)) {}
};

struct SForIn : Stmt {
    std::unique_ptr<Expr> init;
    std::unique_ptr<Expr> object;
    std::unique_ptr<Stmt> body;
    bool isSingleLineBody = false;
    SForIn(std::unique_ptr<Expr> i, std::unique_ptr<Expr> o,
           std::unique_ptr<Stmt> b, SourceRange l)
        : Stmt(StmtKind::ForIn, l),
          init(std::move(i)), object(std::move(o)), body(std::move(b)) {}
};

struct SForOf : Stmt {
    std::unique_ptr<Expr> init;
    std::unique_ptr<Expr> object;
    std::unique_ptr<Stmt> body;
    bool isAwait = false;
    bool isSingleLineBody = false;
    SForOf(std::unique_ptr<Expr> i, std::unique_ptr<Expr> o,
           std::unique_ptr<Stmt> b, bool a, SourceRange l)
        : Stmt(StmtKind::ForOf, l),
          init(std::move(i)), object(std::move(o)), body(std::move(b)), isAwait(a) {}
};

struct SReturn : Stmt {
    std::unique_ptr<Expr> value;
    SReturn(std::unique_ptr<Expr> v, SourceRange l)
        : Stmt(StmtKind::Return, l), value(std::move(v)) {}
};

struct SBreak : Stmt {
    std::string label;
    SBreak(std::string lb, SourceRange l)
        : Stmt(StmtKind::Break, l), label(std::move(lb)) {}
};

struct SContinue : Stmt {
    std::string label;
    SContinue(std::string lb, SourceRange l)
        : Stmt(StmtKind::Continue, l), label(std::move(lb)) {}
};

struct SThrow : Stmt {
    std::unique_ptr<Expr> value;
    SThrow(std::unique_ptr<Expr> v, SourceRange l)
        : Stmt(StmtKind::Throw, l), value(std::move(v)) {}
};

struct SSwitch : Stmt {
    std::unique_ptr<Expr> discriminant;
    struct Case {
        std::unique_ptr<Expr> test;
        StmtList body;
        SourceLoc loc;
    };
    std::vector<Case> cases;
    SourceLoc bodyLoc;
    SourceLoc closeBraceLoc;
    SSwitch(std::unique_ptr<Expr> d, std::vector<Case> c, SourceRange l)
        : Stmt(StmtKind::Switch, l),
          discriminant(std::move(d)), cases(std::move(c)) {}
};

struct CatchClause {
    std::unique_ptr<Binding> param;
    StmtList body;
    SourceLoc loc;
    SourceLoc blockLoc;
};

struct FinallyClause {
    StmtList body;
    SourceLoc loc;
};

struct STry : Stmt {
    StmtList body;
    std::optional<CatchClause> handler;
    std::optional<FinallyClause> finalizer;
    SourceLoc blockLoc;
    STry(StmtList b, std::optional<CatchClause> c, std::optional<FinallyClause> f, SourceRange l)
        : Stmt(StmtKind::Try, l),
          body(std::move(b)), handler(std::move(c)), finalizer(std::move(f)) {}
};

struct SLabel : Stmt {
    std::string name;
    std::unique_ptr<Stmt> stmt;
    bool isSingleLineStmt = false;
    SLabel(std::string n, std::unique_ptr<Stmt> s, SourceRange l)
        : Stmt(StmtKind::Label, l),
          name(std::move(n)), stmt(std::move(s)) {}
};

struct SDebugger : Stmt {
    SDebugger(SourceRange l) : Stmt(StmtKind::Debugger, l) {}
};

struct SWith : Stmt {
    std::unique_ptr<Expr> object;
    std::unique_ptr<Stmt> body;
    SourceLoc bodyLoc;
    bool isSingleLineBody = false;
    SWith(std::unique_ptr<Expr> o, std::unique_ptr<Stmt> b, SourceRange l)
        : Stmt(StmtKind::With, l),
          object(std::move(o)), body(std::move(b)) {}
};

struct SVariable : Stmt {
    VarKind varKind;
    std::vector<Decl> declarations;
    SVariable(VarKind vk, std::vector<Decl> decls, SourceRange l)
        : Stmt(StmtKind::Variable, l),
          varKind(vk), declarations(std::move(decls)) {}
};

struct SFunction : Stmt {
    FnKind fnKind;
    std::string name;
    std::vector<std::unique_ptr<Binding>> params;
    StmtList body;
    bool isExport = false;
    SFunction(FnKind fk, std::string n,
              std::vector<std::unique_ptr<Binding>> p,
              StmtList b, SourceRange l)
        : Stmt(StmtKind::Function, l),
          fnKind(fk), name(std::move(n)),
          params(std::move(p)), body(std::move(b)) {}
};

struct SClass : Stmt {
    std::string name;
    std::unique_ptr<Expr> extends;
    StmtList body;
    bool isExport = false;
    SClass(std::string n, std::unique_ptr<Expr> e, StmtList b, SourceRange l)
        : Stmt(StmtKind::Class, l),
          name(std::move(n)), extends(std::move(e)), body(std::move(b)) {}
};

struct SClassMember : Stmt {
    std::unique_ptr<Expr> key;
    std::unique_ptr<Expr> value;
    bool isStatic = false;
    bool isComputed = false;
    SClassMember(std::unique_ptr<Expr> k, std::unique_ptr<Expr> v,
                 bool s, bool c, SourceRange l)
        : Stmt(StmtKind::ClassMember, l),
          key(std::move(k)), value(std::move(v)), isStatic(s), isComputed(c) {}
};

struct ClauseItem {
    std::string alias;
    std::string originalName;
    SourceLoc aliasLoc;
};

struct SImport : Stmt {
    std::string defaultName;
    std::vector<ClauseItem> items;
    std::string starName;
    std::string modulePath;
    SourceLoc starNameLoc;
    bool isSingleLine = false;
    SImport(std::string path, SourceRange l)
        : Stmt(StmtKind::Import, l), modulePath(std::move(path)) {}
};

struct SExportClause : Stmt {
    std::vector<ClauseItem> items;
    bool isSingleLine = false;
    SExportClause(std::vector<ClauseItem> itms, SourceRange l)
        : Stmt(StmtKind::ExportClause, l), items(std::move(itms)) {}
};

struct SExportDefault : Stmt {
    std::unique_ptr<Stmt> value;
    SExportDefault(std::unique_ptr<Stmt> v, SourceRange l)
        : Stmt(StmtKind::ExportDefault, l), value(std::move(v)) {}
};

struct SExportStar : Stmt {
    std::string modulePath;
    SExportStar(std::string path, SourceRange l)
        : Stmt(StmtKind::ExportStar, l), modulePath(std::move(path)) {}
};

struct SExportFrom : Stmt {
    std::vector<ClauseItem> items;
    std::string modulePath;
    bool isSingleLine = false;
    SExportFrom(std::vector<ClauseItem> itms, std::string path, SourceRange l)
        : Stmt(StmtKind::ExportFrom, l),
          items(std::move(itms)), modulePath(std::move(path)) {}
};

struct SExportEquals : Stmt {
    std::unique_ptr<Expr> value;
    SExportEquals(std::unique_ptr<Expr> v, SourceRange l)
        : Stmt(StmtKind::SExportEquals, l), value(std::move(v)) {}
};

struct SLazyExport : Stmt {
    std::unique_ptr<Expr> value;
    SLazyExport(std::unique_ptr<Expr> v, SourceRange l)
        : Stmt(StmtKind::SLazyExport, l), value(std::move(v)) {}
};

struct SLocal : Stmt {
    VarKind kind;
    bool isExport = false;
    bool isUsing = false;
    bool wasTSImportEquals = false;
    std::vector<Decl> declarations;
    SLocal(VarKind k, std::vector<Decl> d, SourceRange l, bool exp = false, bool use = false)
        : Stmt(StmtKind::SLocal, l),
          kind(k), isExport(exp), isUsing(use), declarations(std::move(d)) {}
};

struct SExport : Stmt {
    std::unique_ptr<Stmt> value;
    SExport(std::unique_ptr<Stmt> v, SourceRange l)
        : Stmt(StmtKind::SExport, l), value(std::move(v)) {}
};

struct TSModuleBlock : Stmt {
    StmtList body;
    TSModuleBlock(StmtList b, SourceRange l)
        : Stmt(StmtKind::TSModuleBlock, l), body(std::move(b)) {}
};

struct TSTypeAlias : Stmt {
    std::string name;
    TSTypeAlias(std::string n, SourceRange l)
        : Stmt(StmtKind::TSTypeAlias, l), name(std::move(n)) {}
};

struct TSInterface : Stmt {
    std::string name;
    TSInterface(std::string n, SourceRange l)
        : Stmt(StmtKind::TSInterface, l), name(std::move(n)) {}
};

struct TSEnum : Stmt {
    std::string name;
    bool isConst = false;
    bool isExport = false;
    struct Member {
        std::string name;
        std::unique_ptr<Expr> value;
    };
    std::vector<Member> members;
    TSEnum(std::string n, bool c, std::vector<Member> m, SourceRange l)
        : Stmt(StmtKind::TSEnum, l),
          name(std::move(n)), isConst(c), members(std::move(m)) {}
};

struct Binding {
    BindingKind kind;
    SourceRange loc;
    Binding(BindingKind k, SourceRange l) : kind(k), loc(l) {}
    virtual ~Binding() = default;
};

struct BIdentifier : Binding {
    std::string name;
    BIdentifier(std::string n, SourceRange l)
        : Binding(BindingKind::Identifier, l), name(std::move(n)) {}
};

struct BArray : Binding {
    std::vector<std::unique_ptr<Binding>> items;
    bool hasSpread = false;
    BArray(std::vector<std::unique_ptr<Binding>> itms, SourceRange l)
        : Binding(BindingKind::Array, l), items(std::move(itms)) {}
};

struct BObject : Binding {
    struct ObjBindingEntry {
        std::unique_ptr<Expr> key;
        std::unique_ptr<Binding> value;
        bool isShorthand = false;
    };
    std::vector<ObjBindingEntry> entries;
    BObject(std::vector<ObjBindingEntry> ents, SourceRange l)
        : Binding(BindingKind::Object, l), entries(std::move(ents)) {}
};

} // namespace guchho::js
