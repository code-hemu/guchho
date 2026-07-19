#pragma once

#include "js_ast_fwd.hpp"
#include <string_view>
#include <string>
#include <cstdint>

namespace guchho::js {

// ── Token types ──────────────────────────────────────────────────────────────

enum class TokenType : uint8_t {
    // Control
    EndOfFile,
    Unexpected,

    // Identifier
    Identifier,

    // Literals
    NumericLiteral,
    StringLiteral,
    BigIntLiteral,
    RegExp,
    NoSubTemplateLiteral,

    // Template literal parts (when interpolation is present)
    TemplateHead,   // `foo${  ...continues
    TemplateMiddle, //  }bar${  ...continues
    TemplateTail,   //  }baz`

    // Keywords
    Abstract,
    As,
    Async,
    Assert,
    Await,
    Break,
    Case,
    Catch,
    Class,
    Const,
    Continue,
    Debugger,
    Default,
    Delete,
    Do,
    Else,
    Enum,
    Export,
    Extends,
    False,
    Finally,
    For,
    From,
    Function,
    Get,
    If,
    Implements,
    Import,
    In,
    Instanceof,
    Interface,
    Let,
    New,
    Null,
    Of,
    Override,
    Package,
    Private,
    Protected,
    Public,
    Return,
    Satisfies,
    Set,
    Static,
    Super,
    Switch,
    This,
    Throw,
    True,
    Try,
    Type,
    Typeof,
    Undefined,
    Var,
    Void,
    While,
    With,
    Yield,

    // Punctuation: single character
    OpenParen,          // (
    CloseParen,         // )
    OpenBracket,        // [
    CloseBracket,       // ]
    OpenBrace,          // {
    CloseBrace,         // }
    Comma,              // ,
    Dot,                // .
    Semicolon,          // ;
    Colon,              // :
    Question,           // ?
    Exclamation,        // !
    Tilde,              // ~
    Plus,               // +
    Minus,              // -
    Star,               // *
    Slash,              // /
    Percent,            // %
    Ampersand,          // &
    Bar,                // |
    Caret,              // ^
    LessThan,           // <
    GreaterThan,        // >

    // Punctuation: multi-character
    DotDotDot,              // ...
    Arrow,                  // =>
    QuestionDot,            // ?.
    QuestionQuestion,       // ??
    QuestionQuestionEquals, // ??=
    ExclamationExclamation, // !!

    // Equality
    EqualsEquals,           // ==
    NotEquals,              // !=
    EqualsEqualsEquals,     // ===
    NotEqualsEquals,        // !==

    // Comparison
    LessThanEquals,         // <=
    GreaterThanEquals,      // >=

    // Shift
    LessThanLessThan,              // <<
    GreaterThanGreaterThan,        // >>
    GreaterThanGreaterThanGreaterThan, // >>>

    // Update
    PlusPlus,   // ++
    MinusMinus, // --

    // Exponentiation
    StarStar,       // **

    // Assignment
    Equals,                     // =
    PlusEquals,                 // +=
    MinusEquals,                // -=
    StarEquals,                 // *=
    SlashEquals,                // /=
    PercentEquals,              // %=
    StarStarEquals,             // **=
    AmpersandEquals,            // &=
    BarEquals,                  // |=
    CaretEquals,                // ^=
    LessThanLessThanEquals,     // <<=
    GreaterThanGreaterThanEquals, // >>=
    TripleGreaterThanEquals,    // >>>=
    LessThanEqualsGreaterThan,  // <=> (spaceship, not JS but for consistency)

    // Logical
    AmpersandAmpersand, // &&
    BarBar,             // ||

    // JSX
    JSXText,
    JSXTagStart,    // < in JSX context (different from LessThan)
    JSXTagEnd,      // > in JSX context
    JSXSelfClose,   // />
};

// ── Token struct ─────────────────────────────────────────────────────────────

struct JsToken {
    TokenType type = TokenType::EndOfFile;
    std::string_view lexeme;
    SourceRange range;
    bool hasNewlineBefore = false;

    bool is(TokenType t) const { return type == t; }
    bool isNot(TokenType t) const { return type != t; }
    bool isKeyword() const {
        return type >= TokenType::Abstract && type <= TokenType::Yield;
    }
    bool isLiteral() const {
        return type >= TokenType::NumericLiteral &&
               type <= TokenType::TemplateTail;
    }
    bool isAssignment() const {
        return type >= TokenType::Equals &&
               type <= TokenType::TripleGreaterThanEquals;
    }
};

// ── Helper: keyword string → TokenType ───────────────────────────────────────

TokenType keywordToTokenType(std::string_view word);

// ── Helper: token name for diagnostics ───────────────────────────────────────

std::string_view tokenTypeName(TokenType t);

} // namespace guchho::js
