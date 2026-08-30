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

[[noreturn]] void unexpected(const Token& token, const std::string& message)
{
    throw std::runtime_error(location(token) + ": " + message +
                             " (found '" + token.value + "')");
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
        throw std::runtime_error(location(token) + ": " + message +
                                 " (found '" + token.value + "')");
    }

    return advance();
}

void Parser::parse()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing source\n";
    }

    current = 0;
    scope_stack.clear();

    parse_root();
    consume(TokenType::EndOfFile, "Expected end of file");

    if (!scope_stack.empty()) {
        throw std::runtime_error(
            location(peek()) + ": Unclosed scope '" + scope_stack.back() + "'"
        );
    }

    scope_stack.clear();
}

void Parser::parse_root()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing root\n";
    }

    consume(TokenType::Colon, "Expected ':root' at beginning of file");

    const Token& root = consume(
        TokenType::Identifier,
        "Expected 'root' after ':'"
    );

    if (root.value != "root") {
        throw std::runtime_error(location(root) + ": Expected ':root'");
    }

    consume(TokenType::LeftBrace, "Expected '{' after ':root'");

    scope_stack.push_back(root.value);
    parse_block();
    scope_stack.pop_back();
}

void Parser::parse_block()
{
    while (!check(TokenType::RightBrace)) {
        if (check(TokenType::EndOfFile)) {
            throw std::runtime_error(
                location(peek()) + ": Expected '}' before end of file"
            );
        }

        if (check(TokenType::Identifier) && peek().value == "var") {
            parse_variable();
            continue;
        }

        if (!check(TokenType::Identifier)) {
            unexpected(peek(), "Expected tag, property, or variable declaration");
        }

        if (current + 1 >= tokens.size()) {
            throw std::runtime_error(
                location(peek()) + ": Expected '{', '<', or ':' after identifier"
            );
        }

        const TokenType next = tokens[current + 1].type;

        if (next == TokenType::LeftBrace || next == TokenType::LeftAngle) {
            parse_tag();
            continue;
        }

        if (next == TokenType::Colon) {
            parse_property();
            continue;
        }

        throw std::runtime_error(
            location(peek()) +
            ": Expected '{' or '<' after tag name (or ':' for a property)"
        );
    }

    consume(TokenType::RightBrace, "Expected '}' after block");
}

void Parser::parse_variable()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing variable declaration\n";
    }

    const Token& var = consume(TokenType::Identifier, "Expected 'var'");

    if (var.value != "var") {
        throw std::runtime_error(location(var) + ": Expected 'var'");
    }

    consume(TokenType::Identifier, "Expected variable name after 'var'");
    consume(TokenType::Equals, "Expected '=' after variable name");
    parse_value("variable declaration");
    consume(TokenType::Semicolon, "Expected ';' after variable declaration");
}

void Parser::parse_tag()
{
    const Token& tag = consume(TokenType::Identifier, "Expected tag name");

    if (csam_debug) {
        std::cout << "Parser: Parsing tag " << tag.value << '\n';
    }

    if (check(TokenType::LeftBrace)) {
        advance();
        scope_stack.push_back(tag.value);
        parse_block();
        scope_stack.pop_back();
        return;
    }

    if (check(TokenType::LeftAngle)) {
        parse_tag_content();

        if (check(TokenType::LeftBrace)) {
            advance();
            scope_stack.push_back(tag.value);
            parse_block();
            scope_stack.pop_back();
        }

        return;
    }

    throw std::runtime_error(
        location(peek()) + ": Expected '{' or '<' after tag name"
    );
}

void Parser::parse_tag_content()
{
    consume(TokenType::LeftAngle, "Expected '<' for tag content");

    if (check(TokenType::RightAngle)) {
        throw std::runtime_error(
            location(peek()) + ": Expected content after '<'"
        );
    }

    while (!check(TokenType::RightAngle)) {
        if (check(TokenType::EndOfFile)) {
            throw std::runtime_error(
                location(peek()) + ": Expected '>' before end of file"
            );
        }

        advance();
    }

    consume(TokenType::RightAngle, "Expected '>' after tag content");
}

void Parser::parse_property()
{
    const Token& property = consume(
        TokenType::Identifier,
        "Expected property name"
    );

    if (csam_debug) {
        std::cout << "Parser: Parsing property " << property.value << '\n';
    }

    consume(TokenType::Colon, "Expected ':' after property name");
    parse_value("property");
    consume(TokenType::Semicolon, "Expected ';' after property value");
}

void Parser::parse_value(const char* context)
{
    if (check(TokenType::Semicolon)) {
        throw std::runtime_error(
            location(peek()) + ": Expected value in " + context
        );
    }

    while (!check(TokenType::Semicolon)) {
        if (check(TokenType::RightBrace) || check(TokenType::EndOfFile)) {
            throw std::runtime_error(
                location(peek()) + ": Expected ';' after " + context
            );
        }

        advance();
    }
}
