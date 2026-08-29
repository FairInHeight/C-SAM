#include "parser.hpp"

#include <stdexcept>
#include <string>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens)
{
}

const Token& Parser::peek() const
{
    return tokens[current];
}

const Token& Parser::advance()
{
    return tokens[current++];
}

bool Parser::check(TokenType type) const
{
    return peek().type == type;
}

const Token& Parser::consume(TokenType type, const char* message)
{
    if (!check(type)) {
        const Token& token = peek();

        throw std::runtime_error(
            std::string(message) +
            " at line " +
            std::to_string(token.line) +
            ", column " +
            std::to_string(token.column)
        );
    }

    return advance();
}

void Parser::parse()
{
    parse_root();

    consume(
        TokenType::EndOfFile,
        "Expected end of file"
    );
}

void Parser::parse_root()
{
    const Token& root = consume(
        TokenType::Identifier,
        "Expected 'root'"
    );

    if (root.value != "root") {
        throw std::runtime_error(
            "Expected 'root' at line " +
            std::to_string(root.line) +
            ", column " +
            std::to_string(root.column)
        );
    }

    consume(
        TokenType::Colon,
        "Expected ':' after root"
    );

    consume(
        TokenType::LeftBrace,
        "Expected '{' after root:"
    );

    while (!check(TokenType::RightBrace)) {
        if (check(TokenType::Identifier) && peek().value == "var") {
            parse_variable();
            continue;
        }

        const Token& token = peek();

        throw std::runtime_error(
            "Unexpected token '" +
            token.value +
            "' in root at line " +
            std::to_string(token.line) +
            ", column " +
            std::to_string(token.column)
        );
    }

    consume(
        TokenType::RightBrace,
        "Expected '}' after root block"
    );
}

void Parser::parse_variable()
{
    const Token& var = consume(
        TokenType::Identifier,
        "Expected 'var'"
    );

    if (var.value != "var") {
        throw std::runtime_error(
            "Expected 'var' at line " +
            std::to_string(var.line) +
            ", column " +
            std::to_string(var.column)
        );
    }

    consume(
        TokenType::Identifier,
        "Expected variable name"
    );

    consume(
        TokenType::Equals,
        "Expected '=' after variable name"
    );

    if (!check(TokenType::String) &&
        !check(TokenType::Number) &&
        !check(TokenType::Hash) &&
        !check(TokenType::Identifier)) {

        const Token& token = peek();

        throw std::runtime_error(
            "Expected value after '=' at line " +
            std::to_string(token.line) +
            ", column " +
            std::to_string(token.column)
        );
    }

    advance();

    consume(
        TokenType::Semicolon,
        "Expected ';' after variable declaration"
    );
}