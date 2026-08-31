#ifndef CSAM_TOKEN_HPP
#define CSAM_TOKEN_HPP

#include <cstddef>
#include <string>

struct SourceLocation {
    std::string filepath;
    std::size_t line;
    std::size_t column;
};

enum class TokenType {
    Identifier,
    String,
    Number,

    Colon,
    Semicolon,
    LeftBrace,
    RightBrace,
    LeftAngle,
    RightAngle,
    LeftBracket,
    RightBracket,
    LeftParen,
    RightParen,
    Equals,

    Hash,
    Comma,

    EndOfFile
};

inline const char* token_type_name(TokenType type)
{
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::String: return "String";
        case TokenType::Number: return "Number";
        case TokenType::Colon: return "Colon";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftAngle: return "LeftAngle";
        case TokenType::RightAngle: return "RightAngle";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::Equals: return "Equals";
        case TokenType::Hash: return "Hash";
        case TokenType::Comma: return "Comma";
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