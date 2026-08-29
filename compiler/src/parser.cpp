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

    // Root contents will be implemented next.

    consume(
        TokenType::RightBrace,
        "Expected '}' after root block"
    );
}