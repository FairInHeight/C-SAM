#include "value_parser.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

ValueParser::ValueParser(const std::vector<Token>& tokens, std::size_t& current)
    : tokens(tokens), current(current)
{
}

const Token& ValueParser::peek() const
{
    if (current >= tokens.size()) {
        throw std::runtime_error("Parser: unexpected end of token stream");
    }

    return tokens[current];
}

const Token& ValueParser::advance()
{
    if (current < tokens.size()) {
        ++current;
    }

    return tokens[current - 1];
}

bool ValueParser::check(TokenType type) const
{
    return current < tokens.size() && tokens[current].type == type;
}

const Token& ValueParser::consume(TokenType type, const char* message)
{
    if (check(type)) {
        return advance();
    }

    unexpected(peek(), message);
}

[[noreturn]] void ValueParser::unexpected(const Token& token, const char* message) const
{
    std::ostringstream error;
    error << "Parser: " << message
          << " at " << token.location.filepath.string()
          << ':' << token.location.line
          << ':' << token.location.column;
    throw std::runtime_error(error.str());
}

std::vector<std::unique_ptr<ValueNode>> ValueParser::parse_value()
{
    std::vector<std::unique_ptr<ValueNode>> values;

    while (current < tokens.size() &&
           !check(TokenType::Semicolon) &&
           !check(TokenType::EndOfFile)) {
        values.push_back(parse_single_value());
    }

    if (values.empty()) {
        unexpected(peek(), "Expected value");
    }

    return values;
}

std::unique_ptr<ValueNode> ValueParser::parse_function()
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

FunctionValueNode::Argument ValueParser::parse_function_argument()
{
    FunctionValueNode::Argument values;

    while (current < tokens.size() &&
           !check(TokenType::Comma) &&
           !check(TokenType::RightParen)) {
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

std::unique_ptr<ValueNode> ValueParser::parse_single_value()
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
