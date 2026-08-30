#ifndef CSAM_PARSER_HPP
#define CSAM_PARSER_HPP

#include "token.hpp"

#include <cstddef>
#include <string>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    void parse();

private:
    const std::vector<Token>& tokens;
    std::size_t current = 0;
    std::vector<std::string> scope_stack;

    const Token& peek() const;
    const Token& advance();
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const char* message);

    void parse_root();
    void parse_block();
    void parse_variable();
    void parse_tag();
    void parse_tag_content();
    void parse_property();
    void parse_value(const char* context);
};

#endif