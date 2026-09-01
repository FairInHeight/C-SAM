#ifndef CSAM_TOKEN_HPP
#define CSAM_TOKEN_HPP

#include <cstddef>
#include <filesystem>
#include <string>

struct SourceLocation {
    std::filesystem::path filepath;
    std::size_t line;
    std::size_t column;
};

enum class TokenType {
    Identifier,
    String,
    Number,
    Percent,
    Hash,
    AtKeyword,

    Colon,
    Semicolon,
    Comma,
    Equals,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    LeftParen,
    RightParen,

    Plus,
    Minus,
    Asterisk,
    Slash,
    GreaterThan,
    LessThan,
    Tilde,
    Pipe,
    Caret,
    Dollar,
    Ampersand,
    Bang,
    QuestionMark,
    Dot,
    Backslash,

    IncludesMatch,
    DashMatch,
    PrefixMatch,
    SuffixMatch,
    SubstringMatch,
    Column,

    EndOfFile
};

inline const char* token_type_name(TokenType type)
{
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::String: return "String";
        case TokenType::Number: return "Number";
        case TokenType::Percent: return "Percent";
        case TokenType::Hash: return "Hash";
        case TokenType::AtKeyword: return "AtKeyword";
        case TokenType::Colon: return "Colon";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Comma: return "Comma";
        case TokenType::Equals: return "Equals";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Asterisk: return "Asterisk";
        case TokenType::Slash: return "Slash";
        case TokenType::GreaterThan: return "GreaterThan";
        case TokenType::LessThan: return "LessThan";
        case TokenType::Tilde: return "Tilde";
        case TokenType::Pipe: return "Pipe";
        case TokenType::Caret: return "Caret";
        case TokenType::Dollar: return "Dollar";
        case TokenType::Ampersand: return "Ampersand";
        case TokenType::Bang: return "Bang";
        case TokenType::QuestionMark: return "QuestionMark";
        case TokenType::Dot: return "Dot";
        case TokenType::Backslash: return "Backslash";
        case TokenType::IncludesMatch: return "IncludesMatch";
        case TokenType::DashMatch: return "DashMatch";
        case TokenType::PrefixMatch: return "PrefixMatch";
        case TokenType::SuffixMatch: return "SuffixMatch";
        case TokenType::SubstringMatch: return "SubstringMatch";
        case TokenType::Column: return "Column";
        case TokenType::EndOfFile: return "EndOfFile";
    }

    return "Unknown";
}

struct Token {
    TokenType type;
    std::string value;
    SourceLocation location;
};

#endif
