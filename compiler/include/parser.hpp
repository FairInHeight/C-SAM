#ifndef CSAM_PARSER_HPP
#define CSAM_PARSER_HPP

#include "ast.hpp"
#include "token.hpp"
#include "value.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<RootNode> parse();

private:
    const std::vector<Token>& tokens;
    std::size_t current = 0;
    std::vector<ASTNode*> scope_stack;

    const Token& peek() const;
    const Token& advance();
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const char* message);

    void validate_delimiters() const;
    void parse_block();
    void parse_variable();
    void parse_tag();
    std::unique_ptr<ContentNode> parse_tag_content();
    void parse_property();
    std::vector<std::unique_ptr<ValueNode>> parse_value(const char* context);
    std::unique_ptr<ValueNode> parse_single_value();
    std::unique_ptr<ValueNode> parse_function();
    ValueNode::Argument parse_function_argument();

    ASTNode* current_scope() const;
    void add_variable(std::unique_ptr<VariableNode> variable);
    void add_property(std::unique_ptr<PropertyNode> property);
    void add_tag(std::unique_ptr<TagNode> tag);
};

#endif
