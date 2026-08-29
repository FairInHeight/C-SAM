#include "parser.hpp"
#include "debug.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string location(const Token& token)
{
    return token.filepath + ":" +
           std::to_string(token.line) + ":" +
           std::to_string(token.column);
}

}

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
            location(token) + ": " + message
        );
    }

    return advance();
}

void Parser::parse()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing source\n";
    }

    parse_root();

    consume(
        TokenType::EndOfFile,
        "Expected end of file"
    );
}

void Parser::parse_root()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing root\n";
    }

    const Token& root = consume(
        TokenType::Identifier,
        "Expected 'root'"
    );

    if (root.value != "root") {
        throw std::runtime_error(
            location(root) + ": Expected 'root'"
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
            location(token) + ": Unexpected token '" +
            token.value + "' in root"
        );
    }

    consume(
        TokenType::RightBrace,
        "Expected '}' after root block"
    );
}

void Parser::parse_variable()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing variable declaration\n";
    }

    const Token& var = consume(
        TokenType::Identifier,
        "Expected 'var'"
    );

    if (var.value != "var") {
        throw std::runtime_error(
            location(var) + ": Expected 'var'"
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
            location(token) + ": Expected value after '='"
        );
    }

    advance();

    consume(
        TokenType::Semicolon,
        "Expected ';' after variable declaration"
    );
}