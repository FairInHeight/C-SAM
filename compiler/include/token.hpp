#ifndef CSAM_TOKEN_HPP
#define CSAM_TOKEN_HPP

#include <string>
#include <cstddef>

enum class TokenType {
    Identifier,
    String,
    Number,

    Colon,
    Semicolon,
    LeftBrace,
    RightBrace,

    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
    std::size_t line;
    std::size_t column;
};

#endif