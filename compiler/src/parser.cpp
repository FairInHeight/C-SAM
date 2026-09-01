#include "parser.hpp"
#include "debug.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

std::string location(const Token& token)
{
    return token.location.filepath.string() + ":" +
           std::to_string(token.location.line) + ":" +
           std::to_string(token.location.column);
}

[[noreturn]] void unexpected(const Token& token, const std::string& message)
{
    throw std::runtime_error(location(token) + ": " + message +
                             " (found '" + token.value + "')");
}

char closing_delimiter(char opener)
{
    switch (opener) {
        case '{': return '}';
        case '<': return '>';
        case '[': return ']';
        case '(': return ')';
        default: return '\0';
    }
}

bool is_opener(TokenType type)
{
    return type == TokenType::LeftBrace ||
           type == TokenType::LessThan ||
           type == TokenType::LeftBracket ||
           type == TokenType::LeftParen;
}

bool is_closer(TokenType type)
{
    return type == TokenType::RightBrace ||
           type == TokenType::GreaterThan ||
           type == TokenType::RightBracket ||
           type == TokenType::RightParen;
}

char delimiter_char(const Token& token)
{
    return token.value.empty() ? '\0' : token.value[0];
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

void Parser::validate_delimiters() const
{
    std::vector<const Token*> delimiters;

    for (const Token& token : tokens) {
        if (is_opener(token.type)) {
            delimiters.push_back(&token);
            continue;
        }

        if (!is_closer(token.type)) {
            continue;
        }

        if (delimiters.empty()) {
            throw std::runtime_error(
                location(token) + ": Unexpected closing delimiter '" +
                std::string(1, delimiter_char(token)) + "'"
            );
        }

        const Token& opener = *delimiters.back();
        const char expected = closing_delimiter(delimiter_char(opener));
        const char actual = delimiter_char(token);

        if (actual != expected) {
            throw std::runtime_error(
                location(token) + ": Unexpected closing delimiter '" +
                std::string(1, actual) + "'; expected '" +
                std::string(1, expected) + "' for '" +
                std::string(1, delimiter_char(opener)) + "' opened at " +
                location(opener)
            );
        }

        delimiters.pop_back();
    }

    if (!delimiters.empty()) {
        const Token& opener = *delimiters.back();
        throw std::runtime_error(
            location(opener) + ": Unterminated delimiter '" +
            std::string(1, delimiter_char(opener)) + "'; expected '" +
            std::string(1, closing_delimiter(delimiter_char(opener))) + "'"
        );
    }
}

std::unique_ptr<RootNode> Parser::parse()
{
    if (csam_debug) {
        std::cout << "Parser: Parsing source\n";
    }

    current = 0;
    scope_stack.clear();

    validate_delimiters();

    if (check(TokenType::EndOfFile)) {
        unexpected(peek(), "Expected ':root' at beginning of file");
    }

    consume(TokenType::Colon, "Expected ':root' at beginning of file");

    const Token& root_token = consume(
        TokenType::Identifier,
        "Expected 'root' after ':'"
    );

    if (root_token.value != "root") {
        throw std::runtime_error(location(root_token) + ": Expected ':root'");
    }

    auto root = std::make_unique<RootNode>(root_token);

    consume(TokenType::LeftBrace, "Expected '{' after ':root'");

    scope_stack.push_back(root.get());
    parse_block();
    scope_stack.pop_back();

    consume(TokenType::EndOfFile, "Expected end of file");

    if (!scope_stack.empty()) {
        throw std::runtime_error(
            location(peek()) + ": Unclosed scope"
        );
    }

    if (csam_debug) {
        std::cout << "Parser: Finished parsing source\n";
    }

    return root;
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
                location(peek()) + ": Expected '{' or '<' after tag name"
            );
        }

        const TokenType next = tokens[current + 1].type;

        if (next == TokenType::LeftBrace || next == TokenType::LessThan) {
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

ASTNode* Parser::current_scope() const
{
    if (scope_stack.empty()) {
        throw std::runtime_error("Parser: No active scope");
    }

    return scope_stack.back();
}

void Parser::add_variable(std::unique_ptr<VariableNode> variable)
{
    ASTNode* parent = current_scope();

    if (parent->type() == ASTNodeType::Root) {
        static_cast<RootNode*>(parent)->add_variable(std::move(variable));
        return;
    }

    if (parent->type() == ASTNodeType::Tag) {
        static_cast<TagNode*>(parent)->add_variable(std::move(variable));
        return;
    }

    throw std::runtime_error("Parser: Invalid variable scope");
}

void Parser::add_property(std::unique_ptr<PropertyNode> property)
{
    ASTNode* parent = current_scope();

    if (parent->type() == ASTNodeType::Root) {
        static_cast<RootNode*>(parent)->add_property(std::move(property));
        return;
    }

    if (parent->type() == ASTNodeType::Tag) {
        static_cast<TagNode*>(parent)->add_property(std::move(property));
        return;
    }

    throw std::runtime_error("Parser: Invalid property scope");
}

void Parser::add_tag(std::unique_ptr<TagNode> tag)
{
    ASTNode* parent = current_scope();

    if (parent->type() == ASTNodeType::Root) {
        static_cast<RootNode*>(parent)->add_tag(std::move(tag));
        return;
    }

    if (parent->type() == ASTNodeType::Tag) {
        static_cast<TagNode*>(parent)->add_child(std::move(tag));
        return;
    }

    throw std::runtime_error("Parser: Invalid tag scope");
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

    const Token& name = consume(
        TokenType::Identifier,
        "Expected variable name after 'var'"
    );

    consume(TokenType::Equals, "Expected '=' after variable name");
    std::vector<Token> value = parse_value("variable declaration");
    consume(TokenType::Semicolon, "Expected ';' after variable declaration");

    add_variable(std::make_unique<VariableNode>(name, std::move(value)));
}

void Parser::parse_tag()
{
    const Token& tag_token = consume(TokenType::Identifier, "Expected tag name");

    if (csam_debug) {
        std::cout << "Parser: Parsing tag " << tag_token.value << '\n';
    }

    auto tag = std::make_unique<TagNode>(tag_token);
    TagNode* tag_ptr = tag.get();

    if (check(TokenType::LeftBrace)) {
        advance();
        add_tag(std::move(tag));

        scope_stack.push_back(tag_ptr);
        parse_block();
        scope_stack.pop_back();
        return;
    }

    if (check(TokenType::LessThan)) {
        std::unique_ptr<ContentNode> content = parse_tag_content();
        tag->set_content(std::move(content));

        const bool has_block = check(TokenType::LeftBrace);
        add_tag(std::move(tag));

        if (has_block) {
            advance();
            scope_stack.push_back(tag_ptr);
            parse_block();
            scope_stack.pop_back();
        }

        return;
    }

    throw std::runtime_error(
        location(peek()) + ": Expected '{' or '<' after tag name"
    );
}

std::unique_ptr<ContentNode> Parser::parse_tag_content()
{
    const Token& left_angle = consume(
        TokenType::LessThan,
        "Expected '<' for tag content"
    );

    auto content = std::make_unique<ContentNode>(left_angle);

    if (check(TokenType::GreaterThan)) {
        throw std::runtime_error(
            location(peek()) + ": Expected content after '<'"
        );
    }

    while (!check(TokenType::GreaterThan)) {
        if (check(TokenType::EndOfFile)) {
            throw std::runtime_error(
                location(peek()) + ": Expected '>' before end of file"
            );
        }

        content->add_token(advance());
    }

    consume(TokenType::GreaterThan, "Expected '>' after tag content");
    return content;
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
    std::vector<Token> value = parse_value("property");
    consume(TokenType::Semicolon, "Expected ';' after property value");

    add_property(std::make_unique<PropertyNode>(property, std::move(value)));
}

std::vector<Token> Parser::parse_value(const char* context)
{
    if (check(TokenType::Semicolon)) {
        throw std::runtime_error(
            location(peek()) + ": Expected value in " + context
        );
    }

    std::vector<Token> value;

    while (!check(TokenType::Semicolon)) {
        if (check(TokenType::RightBrace) || check(TokenType::EndOfFile)) {
            throw std::runtime_error(
                location(peek()) + ": Expected ';' after " + context
            );
        }

        value.push_back(advance());
    }

    return value;
}
