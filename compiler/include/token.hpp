#ifndef CSAM_TOKEN_HPP
#define CSAM_TOKEN_HPP

#include <cstddef>
#include <string>

enum class TokenType {
    Identifier,
    String,
    Number,

    Colon,
    Semicolon,
    LeftBrace,
    RightBrace,
    Equals,

    Hash,
    Comma,

    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
    std::size_t line;
    std::size_t column;
};

#endif