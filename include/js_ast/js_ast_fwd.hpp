#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>

namespace guchho::js {

struct SourceLoc {
    uint32_t offset = 0;
    uint32_t line = 1;
    uint32_t column = 1;
};

struct SourceRange {
    SourceLoc start;
    SourceLoc end;
};

// ── Precedence levels (matching Go L* constants) ──────────────────────────
enum class Precedence : uint8_t {
    Lowest, Comma, Spread, Yield, Assign, Conditional,
    NullishCoalescing, LogicalOr, LogicalAnd, BitwiseOr,
    BitwiseXor, BitwiseAnd, Equals, Compare, Shift, Add,
    Multiply, Exponentiation, Prefix, Postfix, New, Call, Member,
};

// ── Unified operator code (matching Go OpCode) ───────────────────────────
enum class OpCode : uint8_t {
    // Prefix
    UnOpPos, UnOpNeg, UnOpCpl, UnOpNot, UnOpVoid, UnOpTypeof, UnOpDelete,
    // Prefix update
    UnOpPreDec, UnOpPreInc,
    // Postfix update
    UnOpPostDec, UnOpPostInc,
    // Left-associative binary
    BinOpAdd, BinOpSub, BinOpMul, BinOpDiv, BinOpRem, BinOpPow,
    BinOpLt, BinOpLe, BinOpGt, BinOpGe, BinOpIn, BinOpInstanceof,
    BinOpShl, BinOpShr, BinOpUShr,
    BinOpLooseEq, BinOpLooseNe, BinOpStrictEq, BinOpStrictNe,
    BinOpNullishCoalescing,
    BinOpLogicalOr, BinOpLogicalAnd,
    BinOpBitwiseOr, BinOpBitwiseAnd, BinOpBitwiseXor,
    // Non-associative
    BinOpComma,
    // Right-associative assignment
    BinOpAssign,
    BinOpAddAssign, BinOpSubAssign, BinOpMulAssign, BinOpDivAssign, BinOpRemAssign, BinOpPowAssign,
    BinOpShlAssign, BinOpShrAssign, BinOpUShrAssign,
    BinOpBitwiseOrAssign, BinOpBitwiseAndAssign, BinOpBitwiseXorAssign,
    BinOpNullishCoalescingAssign, BinOpLogicalOrAssign, BinOpLogicalAndAssign,
};

enum class AssignTarget : uint8_t {
    None, Replace, Update,
};

// ── Expr/Stmt kind enums ──────────────────────────────────────────────────
enum class ExprKind : uint8_t {
    Array, Arrow, Await, Binary, Boolean, Call, Class,
    Conditional, Dot, Function, Identifier, Index, Import,
    ImportCall, JSXElement, JSXFragment, New, Null, Number,
    Object, RegExp, Sequence, Spread, String, Super, Template,
    This, Unary, Undefined, Yield, Missing, ExportDefault,
    ExportFrom, ExportStar, BigInt, OptionalChain, PrivateMember,
    EInlinedEnum, EJSXText, ENamespaceImport, ERequireCall,
    EImportMeta, ENewTarget, EMetaProperty, PrivateBrandCheck,
    EImportIdentifier, EPrivateIdentifier, ENameOfSymbol,
    EAnnotation, EIf, ERequireString, ERequireResolveString, EImportString,
};

enum class StmtKind : uint8_t {
    Block, Break, Class, ClassMember, Continue, Debugger,
    DoWhile, Empty, ExportClause, ExportDefault, ExportStar,
    Expression, For, ForIn, ForOf, Function, If, Import,
    Label, Return, Switch, Throw, Try, Variable, While, With,
    ExportFrom, TSModuleBlock, TSTypeAlias, TSInterface, TSEnum,
    SLocal, SExport, SComment, SDirective, STypeScript,
    SExportEquals, SLazyExport,
};

enum class BindingKind : uint8_t {
    Identifier, Array, Object,
};

enum class VarKind : uint8_t {
    Var, Let, Const,
};

enum class FnKind : uint8_t {
    Normal, Async, Generator, AsyncGenerator,
};

enum class ExportKind : uint8_t {
    Named, Default, Star,
};

enum class ImportKind : uint8_t {
    Default, Named, Namespace,
};

// ── New enums from Go ─────────────────────────────────────────────────────
enum class PropertyKind : uint8_t {
    Field, Method, Getter, Setter, AutoAccessor, Spread,
    DeclareOrAbstract, ClassStaticBlock,
};

enum class PropertyFlags : uint8_t {
    None = 0,
    IsComputed = 1 << 0,
    IsStatic = 1 << 1,
    WasShorthand = 1 << 2,
    PreferQuotedKey = 1 << 3,
};

inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b) {
    return static_cast<PropertyFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline PropertyFlags operator&(PropertyFlags a, PropertyFlags b) {
    return static_cast<PropertyFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
inline bool hasFlag(PropertyFlags f, PropertyFlags flag) {
    return (static_cast<uint8_t>(f) & static_cast<uint8_t>(flag)) != 0;
}

enum class CallKind : uint8_t {
    Normal, DirectEval, TargetWasOriginallyPropertyAccess,
};

enum class OptionalChain : uint8_t {
    None, Start, Continue,
};

enum class AnnotationFlags : uint8_t {
    None = 0,
    CanBeRemovedIfUnused = 1 << 0,
};

inline AnnotationFlags operator|(AnnotationFlags a, AnnotationFlags b) {
    return static_cast<AnnotationFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline AnnotationFlags operator&(AnnotationFlags a, AnnotationFlags b) {
    return static_cast<AnnotationFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
inline bool hasFlag(AnnotationFlags f, AnnotationFlags flag) {
    return (static_cast<uint8_t>(f) & static_cast<uint8_t>(flag)) != 0;
}

enum class LocalKind : uint8_t {
    Var, Let, Const, Using, AwaitUsing,
};

enum class StrictModeKind : uint8_t {
    Sloppy, Explicit, ImplicitClass, ImplicitESM,
    ImplicitTSAlwaysStrict, ImplicitJSXAutomaticRuntime,
};

enum class ExportsKind : uint8_t {
    None, CommonJS, ESM, ESMWithDynamicFallback,
};

enum class ModuleType : uint8_t {
    Unknown,
    CommonJS_CJS, CommonJS_CTS, CommonJS_PackageJSON,
    ESM_MJS, ESM_MTS, ESM_PackageJSON,
};

enum class ConstValueKind : uint8_t {
    None, Null, Undefined, True, False, Number, String,
};

enum class SideEffects : uint8_t {
    Unknown, HasSideEffects, NoSideEffects,
};

// ── Forward declarations ──────────────────────────────────────────────────
struct Expr;
struct Stmt;
struct Binding;
using StmtList = std::vector<std::unique_ptr<Stmt>>;

// ── Decorator (from Go) ───────────────────────────────────────────────────
struct Decorator {
    std::unique_ptr<Expr> value;
    SourceLoc atLoc;
    bool omitNewlineAfter = false;
};

// ── ClassStaticBlock (from Go) ────────────────────────────────────────────
struct ClassStaticBlock {
    std::unique_ptr<Stmt> block;
    SourceLoc loc;
};

// ── Property (expanded from Go) ───────────────────────────────────────────
struct Property {
    std::unique_ptr<Expr> key;
    std::unique_ptr<Expr> valueOrNil;
    std::unique_ptr<Expr> initializerOrNil;
    std::vector<Decorator> decorators;
    std::unique_ptr<ClassStaticBlock> classStaticBlock;
    SourceLoc loc;
    SourceLoc closeBracketLoc;
    PropertyKind kind = PropertyKind::Field;
    PropertyFlags flags = PropertyFlags::None;

    bool isShorthand() const { return hasFlag(flags, PropertyFlags::WasShorthand); }
    bool isComputed() const { return hasFlag(flags, PropertyFlags::IsComputed); }
    bool isStatic() const { return hasFlag(flags, PropertyFlags::IsStatic); }
    bool isSpread() const { return kind == PropertyKind::Spread; }
    bool isMethod() const { return kind == PropertyKind::Method || kind == PropertyKind::Getter || kind == PropertyKind::Setter; }
};

// ── Arg (expanded) ────────────────────────────────────────────────────────
struct Arg {
    std::unique_ptr<Binding> binding;
    std::unique_ptr<Expr> defaultOrNil;
    std::vector<Decorator> decorators;
    bool isTypeScriptCtorField = false;
};

// ── Decl ──────────────────────────────────────────────────────────────────
struct Decl {
    std::unique_ptr<Binding> binding;
    std::unique_ptr<Expr> init;
};

// ── ConstValue (from Go) ──────────────────────────────────────────────────
struct ConstValue {
    double number = 0;
    ConstValueKind kind = ConstValueKind::None;
};

// ── Symbol / Scope types (expanded) ───────────────────────────────────────
enum class SymbolKind : uint8_t {
    None, Var, Let, Const, Function, Class, Import, Type, Label,
    Hoisted, HoistedFunction, CatchIdentifier, Arguments,
    PrivateGet, PrivateSet, PrivateGetSetPair,
    PrivateStaticGet, PrivateStaticSet, PrivateStaticGetSetPair,
    Unbound, Other, TSNamespace, TSEnum,
};

enum class ScopeKind : uint8_t {
    Module, Function, Block, Loop, Switch, Class, Catch,
    FunctionArgs, FunctionBody, With, Label, ClassName, ClassBody,
    CatchBinding, Entry, ClassStaticInit,
};

struct Symbol {
    std::string name;
    SymbolKind kind = SymbolKind::None;
    uint32_t id = 0;
    uint32_t useCount = 0;
    bool isUsed = false;
    bool isHoisted() const;
    bool isFunction() const;
};

struct ScopeMember {
    Symbol* symbol = nullptr;
    SourceLoc loc;
};

struct Scope {
    ScopeKind kind;
    Scope* parent = nullptr;
    std::vector<std::unique_ptr<Scope>> children;
    std::unordered_map<std::string, ScopeMember> members;
    std::vector<ScopeMember> generated;
    std::vector<ScopeMember> replaced;
    bool containsDirectEval = false;
    bool forbidArguments = false;
    bool isAfterConstLocalPrefix = false;
    StrictModeKind strictMode = StrictModeKind::Sloppy;
    SourceLoc useStrictLoc;

    Scope(ScopeKind k, Scope* p = nullptr) : kind(k), parent(p) {}

    Symbol* declareSymbol(const std::string& name, SymbolKind symKind, SourceLoc loc);
    Symbol* lookupSymbol(const std::string& name);
    ScopeMember* lookupMember(const std::string& name);
    bool stopsHoisting() const;
    void recursiveSetStrictMode(StrictModeKind k);
};

struct ScopeTree {
    std::unique_ptr<Scope> root;
    ScopeTree() : root(std::make_unique<Scope>(ScopeKind::Module, nullptr)) {}
};

struct ParseError {
    std::string message;
    SourceLoc loc;
};

// ── AST-level types (from Go) ─────────────────────────────────────────────
struct NamedImport {
    std::string alias;
    SourceLoc aliasLoc;
    bool aliasIsStar = false;
    bool isExported = false;
};

struct NamedExport {
    uint32_t ref = 0;
    SourceLoc aliasLoc;
};

struct DeclaredSymbol {
    uint32_t ref = 0;
    bool isTopLevel = false;
};

struct Dependency {
    uint32_t sourceIndex = 0;
    uint32_t partIndex = 0;
};

struct Part {
    StmtList stmts;
    std::vector<Scope*> scopes;
    std::vector<uint32_t> importRecordIndices;
    std::vector<DeclaredSymbol> declaredSymbols;
    std::vector<Dependency> dependencies;
    bool canBeRemovedIfUnused = false;
    bool forceTreeShaking = false;
    bool isLive = false;
};

struct AST {
    std::vector<Part> parts;
    Scope* moduleScope = nullptr;
    ExportsKind exportsKind = ExportsKind::None;
};

} // namespace guchho::js
