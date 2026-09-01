#include "parser.hpp"

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

[[noreturn]] void Parser::unexpected(const Token& token, const char* message) const
{
    std::ostringstream error;
    error << "Parser: " << message
          << " at " << token.location.filepath.string()
          << ':' << token.location.line
          << ':' << token.location.column;
    throw std::runtime_error(error.str());
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
        parent->add_child(std::move(tag));
        return;
    }

    throw std::runtime_error("Parser: invalid scope for tag");
}

std::unique_ptr<RootNode> Parser::parse()
{
    validate_delimiters();

    if (tokens.empty() || check(TokenType::EndOfFile)) {
        unexpected(peek(), "Expected ':root' block");
    }

    // C-SAM's root declaration is written as :root { ... }.
    const Token& root_token = peek();
    if (!check(TokenType::Colon)) {
        unexpected(root_token, "Expected ':' before root name");
    }
    advance();

    const Token& root_name = consume(TokenType::Identifier, "Expected root name");
    if (root_name.value != "root") {
        unexpected(root_name, "Expected root name 'root'");
    }

    consume(TokenType::LeftBrace, "Expected '{' after root declaration");

    auto root = std::make_unique<RootNode>(root_token);
    scope_stack.push_back(root.get());

    while (!check(TokenType::RightBrace)) {
        if (check(TokenType::EndOfFile)) {
            unexpected(peek(), "Expected '}' after root block");
        }

        if (check(TokenType::Identifier)) {
            if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Equals) {
                parse_variable();
            } else if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::LeftBrace) {
                parse_tag();
            } else if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Colon) {
                parse_property();
            } else {
                unexpected(peek(), "Expected variable, tag, or property in root block");
            }
        } else {
            unexpected(peek(), "Unexpected token in root block");
        }
    }

    advance();
    if (!check(TokenType::EndOfFile)) {
        unexpected(peek(), "Unexpected token after root block");
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
    unexpected(peek(), "At-rules are not yet supported by the AST");
}

void Parser::parse_variable()
{
    const Token& name = consume(TokenType::Identifier, "Expected variable name");
    consume(TokenType::Equals, "Expected '=' after variable name");
    auto value = parse_value("variable");
    consume(TokenType::Semicolon, "Expected ';' after variable value");

    add_variable(std::make_unique<VariableNode>(name, std::move(value)));
}

void Parser::parse_tag()
{
    const Token& name = consume(TokenType::Identifier, "Expected tag name");
    consume(TokenType::LeftBrace, "Expected '{' after tag name");

    auto tag = std::make_unique<TagNode>(name);
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
        } else if (check(TokenType::String) || check(TokenType::LessThan)) {
            auto content = parse_tag_content();
            tag_ptr->set_content(std::move(content));
        } else {
            unexpected(peek(), "Unexpected token in tag");
        }
    }

    advance();
    scope_stack.pop_back();
}

std::unique_ptr<ContentNode> Parser::parse_tag_content()
{
    if (check(TokenType::String)) {
        const Token& token = advance();
        return std::make_unique<ContentNode>(token);
    }

    const Token& left = consume(TokenType::LessThan, "Expected '<' before tag content");
    const Token& content = consume(TokenType::Identifier, "Expected tag content");
    consume(TokenType::GreaterThan, "Expected '>' after tag content");

    auto node = std::make_unique<ContentNode>(left);
    node->add_token(content);
    return node;
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
        return std::make_unique<FunctionValueNode>(name, name.value, std::move(arguments));
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

    return std::make_unique<FunctionValueNode>(name, name.value, std::move(arguments));
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
        tokens[current + 1].type == TokenType::LeftParen &&
        token.location.line == tokens[current + 1].location.line &&
        token.location.column + token.value.size() == tokens[current + 1].location.column) {
        return parse_function();
    }

    if (token.type == TokenType::Number) {
        const Token& number = advance();

        if (check(TokenType::Percent)) {
            const Token& percent = peek();
            if (number.location.line == percent.location.line &&
                number.location.column + number.value.size() == percent.location.column) {
                advance();
                return std::make_unique<PercentageValueNode>(number, number.value);
            }
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
