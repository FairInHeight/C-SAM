#ifndef CSAM_VALUE_PARSER_HPP
#define CSAM_VALUE_PARSER_HPP

#include "token.hpp"
#include "value.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class ValueParser {
public:
    ValueParser(const std::vector<Token>& tokens, std::size_t& current);

    std::vector<std::unique_ptr<ValueNode>> parse_value();

private:
    const std::vector<Token>& tokens;
    std::size_t& current;

    const Token& peek() const;
    const Token& advance();
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const char* message);
    [[noreturn]] void unexpected(const Token& token, const char* message) const;

    std::unique_ptr<ValueNode> parse_single_value();
    std::unique_ptr<ValueNode> parse_function();
    FunctionValueNode::Argument parse_function_argument();
};

#endif
