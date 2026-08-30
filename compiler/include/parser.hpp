#ifndef CSAM_PARSER_HPP
#define CSAM_PARSER_HPP

#include "token.hpp"

#include <cstddef>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    void parse();

private:
    const std::vector<Token>& tokens;
    std::size_t current = 0;

    const Token& peek() const;
    const Token& advance();
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const char* message);

    void parse_root();
    void parse_variable();
    void parse_tag();
    void parse_tag_content();
    void parse_block();
    void parse_property();
};

#endif