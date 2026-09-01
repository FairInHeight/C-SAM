#include "parser.hpp"

#include "debug.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

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
    if (current < tokens.size()) {
        ++current;
    }

    return tokens[current - 1];
}

bool Parser::check(TokenType type) const
{
    return peek().type == type;
}

const Token& Parser::consume(TokenType type, const char* message)
{
    if (check(type)) {
        return advance();
    }

    unexpected(peek(), message);
}

ASTNode* Parser::current_scope() const
{
    return scope_stack.empty() ? nullptr : scope_stack.back();
}

void Parser::add_variable(std::unique_ptr<VariableNode> variable)
{
    ASTNode* scope = current_scope();
    if (!scope) {
        throw std::runtime_error("Parser: no active scope for variable");
    }

    if (auto* root = dynamic_cast<RootNode*>(scope)) {
        root->add_variable(std::move(variable));
        return;
    }

    if (auto* tag = dynamic_cast<TagNode*>(scope)) {
        tag->add_variable(std::move(variable));
        return;
    }

    if (auto* content = dynamic_cast<ContentNode*>(scope)) {
        content->add_variable(std::move(variable));
        return;
    }

    throw std::runtime_error("Parser: invalid scope for variable");
}

void Parser::add_property(std::unique_ptr<PropertyNode> property)
{
    ASTNode* scope = current_scope();
    if (!scope) {
        throw std::runtime_error("Parser: no active scope for property");
    }

    if (auto* root = dynamic_cast<RootNode*>(scope)) {
        root->add_property(std::move(property));
        return;
    }

    if (auto* tag = dynamic_cast<TagNode*>(scope)) {
        tag->add_property(std::move(property));
        return;
    }

    if (auto* content = dynamic_cast<ContentNode*>(scope)) {
        content->add_property(std::move(property));
        return;
    }

    throw std::runtime_error("Parser: invalid scope for property");
}

void Parser::add_tag(std::unique_ptr<TagNode> tag)
{
    ASTNode* scope = current_scope();
    if (!scope) {
        throw std::runtime_error("Parser: no active scope for tag");
    }

    if (auto* root = dynamic_cast<RootNode*>(scope)) {
        root->add_tag(std::move(tag));
        return;
    }

    if (auto* parent = dynamic_cast<TagNode*>(scope)) {
        parent->add_tag(std::move(tag));
        return;
    }

    if (auto* content = dynamic_cast<ContentNode*>(scope)) {
        content->add_tag(std::move(tag));
        return;
    }

    throw std::runtime_error("Parser: invalid scope for tag");
}

std::unique_ptr<RootNode> Parser::parse()
{
    validate_delimiters();

    const Token& root_token = tokens.front();
    auto root = std::make_unique<RootNode>(root_token);

    scope_stack.push_back(root.get());
    while (!check(TokenType::EndOfFile)) {
        if (check(TokenType::AtKeyword)) {
            parse_block();
        } else if (check(TokenType::Identifier)) {
            if (current + 1 < tokens.size() &&
                tokens[current + 1].type == TokenType::Equals) {
                parse_variable();
            } else if (current + 1 < tokens.size() &&
                       tokens[current + 1].type == TokenType::LeftBrace) {
                parse_tag();
            } else if (current + 1 < tokens.size() &&
                       tokens[current + 1].type == TokenType::Colon) {
                parse_property();
            } else {
                unexpected(peek(), "Expected variable, tag, or property");
            }
        } else {
            unexpected(peek(), "Unexpected token at top level");
        }
    }
    scope_stack.pop_back();

    return root;
}

void Parser::validate_delimiters() const
{
    std::vector<TokenType> stack;

    for (const Token& token : tokens) {
        switch (token.type) {
        case TokenType::LeftBrace:
        case TokenType::LeftBracket:
        case TokenType::LeftParen:
            stack.push_back(token.type);
            break;

        case TokenType::RightBrace:
            if (stack.empty() || stack.back() != TokenType::LeftBrace) {
                unexpected(token, "Unmatched '}'");
            }
            stack.pop_back();
            break;

        case TokenType::RightBracket:
            if (stack.empty() || stack.back() != TokenType::LeftBracket) {
                unexpected(token, "Unmatched ']'");
            }
            stack.pop_back();
            break;

        case TokenType::RightParen:
            if (stack.empty() || stack.back() != TokenType::LeftParen) {
                unexpected(token, "Unmatched ')'");
            }
            stack.pop_back();
            break;

        default:
            break;
        }
    }

    if (!stack.empty()) {
        throw std::runtime_error("Parser: unmatched opening delimiter");
    }
}

void Parser::parse_block()
{
    const Token& at_keyword = advance();
    const Token& left_brace = consume(TokenType::LeftBrace, "Expected '{' after at-keyword");
    (void)left_brace;

    auto block = std::make_unique<ContentNode>(at_keyword);
    ContentNode* block_ptr = block.get();

    ASTNode* scope = current_scope();
    if (auto* root = dynamic_cast<RootNode*>(scope)) {
        root->add_block(std::move(block));
    } else {
        throw std::runtime_error("Parser: invalid scope for block");
    }

    scope_stack.push_back(block_ptr);
    while (!check(TokenType::RightBrace)) {
        if (check(TokenType::EndOfFile)) {
            unexpected(peek(), "Expected '}' after block");
        }

        if (check(TokenType::Identifier)) {
            if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Equals) {
                parse_variable();
            } else if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::LeftBrace) {
                parse_tag();
            } else if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Colon) {
                parse_property();
            } else {
                unexpected(peek(), "Expected variable, tag, or property in block");
            }
        } else {
            unexpected(peek(), "Unexpected token in block");
        }
    }

    advance();
    scope_stack.pop_back();
}

void Parser::parse_variable()
{
    const Token& name = consume(TokenType::Identifier, "Expected variable name");
    consume(TokenType::Equals, "Expected '=' after variable name");
    auto value = parse_value("variable");
    consume(TokenType::Semicolon, "Expected ';' after variable value");

    add_variable(std::make_unique<VariableNode>(name, name.value, std::move(value)));
}

void Parser::parse_tag()
{
    const Token& name = consume(TokenType::Identifier, "Expected tag name");
    consume(TokenType::LeftBrace, "Expected '{' after tag name");

    auto tag = std::make_unique<TagNode>(name, name.value);
    TagNode* tag_ptr = tag.get();
    add_tag(std::move(tag));

    scope_stack.push_back(tag_ptr);
    while (!check(TokenType::RightBrace)) {
        if (check(TokenType::EndOfFile)) {
            unexpected(peek(), "Expected '}' after tag");
        }

        if (check(TokenType::Identifier)) {
            if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Equals) {
                parse_variable();
            } else if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::LeftBrace) {
                parse_tag();
            } else if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Colon) {
                parse_property();
            } else {
                unexpected(peek(), "Expected variable, tag, or property in tag");
            }
        } else if (check(TokenType::String)) {
            auto content = parse_tag_content();
            tag_ptr->add_content(std::move(content));
        } else {
            unexpected(peek(), "Unexpected token in tag");
        }
    }

    advance();
    scope_stack.pop_back();
}

std::unique_ptr<ContentNode> Parser::parse_tag_content()
{
    const Token& token = consume(TokenType::String, "Expected string content");
    return std::make_unique<ContentNode>(token);
}

void Parser::parse_property()
{
    const Token& property = consume(TokenType::Identifier, "Expected property name");
    consume(TokenType::Colon, "Expected ':' after property name");
    auto value = parse_value("property");
    consume(TokenType::Semicolon, "Expected ';' after property value");

    add_property(std::make_unique<PropertyNode>(property, std::move(value)));
}

std::vector<std::unique_ptr<ValueNode>> Parser::parse_value(const char* context)
{
    (void)context;
    std::vector<std::unique_ptr<ValueNode>> values;

    while (!check(TokenType::Semicolon) && !check(TokenType::EndOfFile)) {
        values.push_back(parse_single_value());
    }

    if (values.empty()) {
        unexpected(peek(), "Expected value");
    }

    return values;
}

std::unique_ptr<ValueNode> Parser::parse_function()
{
    const Token& name = consume(TokenType::Identifier, "Expected function name");
    consume(TokenType::LeftParen, "Expected '(' after function name");

    std::vector<FunctionValueNode::Argument> arguments;

    if (check(TokenType::RightParen)) {
        advance();
        return std::make_unique<FunctionValueNode>(
            name,
            name.value,
            std::move(arguments)
        );
    }

    while (true) {
        arguments.push_back(parse_function_argument());

        if (check(TokenType::RightParen)) {
            advance();
            break;
        }

        consume(TokenType::Comma, "Expected ',' or ')' in function arguments");

        if (check(TokenType::RightParen)) {
            unexpected(peek(), "Expected function argument after ','");
        }
    }

    return std::make_unique<FunctionValueNode>(
        name,
        name.value,
        std::move(arguments)
    );
}

FunctionValueNode::Argument Parser::parse_function_argument()
{
    FunctionValueNode::Argument values;

    while (!check(TokenType::Comma) && !check(TokenType::RightParen)) {
        if (check(TokenType::EndOfFile) || check(TokenType::Semicolon)) {
            unexpected(peek(), "Expected ')' after function arguments");
        }

        values.push_back(parse_single_value());
    }

    if (values.empty()) {
        unexpected(peek(), "Expected function argument");
    }

    return values;
}

std::unique_ptr<ValueNode> Parser::parse_single_value()
{
    const Token& token = peek();

    if (token.type == TokenType::Identifier &&
        current + 1 < tokens.size() &&
        tokens[current + 1].type == TokenType::LeftParen) {
        return parse_function();
    }

    if (token.type == TokenType::Number) {
        const Token& number = advance();

        if (check(TokenType::Percent)) {
            const Token& percent = advance();
            if (number.location.line == percent.location.line &&
                number.location.column + number.value.size() == percent.location.column) {
                return std::make_unique<PercentageValueNode>(number, number.value);
            }
            return std::make_unique<RawValueNode>(number, number.value);
        }

        if (check(TokenType::Identifier)) {
            const Token& unit = peek();
            if (number.location.line == unit.location.line &&
                number.location.column + number.value.size() == unit.location.column) {
                advance();
                return std::make_unique<DimensionValueNode>(number, number.value, unit.value);
            }
        }

        return std::make_unique<NumberValueNode>(number, number.value);
    }

    if (token.type == TokenType::String) {
        return std::make_unique<StringValueNode>(advance(), token.value);
    }

    return std::make_unique<RawValueNode>(advance(), token.value);
}
