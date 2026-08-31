#ifndef CSAM_AST_HPP
#define CSAM_AST_HPP

#include "token.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

enum class ASTNodeType {
    Root,
    Tag,
    Content,
    Property,
    Variable
};

class ASTNode {
public:
    ASTNode(ASTNodeType type, const Token& token);
    virtual ~ASTNode() = default;

    ASTNodeType type() const { return node_type; }
    const SourceLocation& location() const { return source_location; }

private:
    ASTNodeType node_type;
    SourceLocation source_location;
};

class ContentNode final : public ASTNode {
public:
    explicit ContentNode(const Token& token);

    void add_token(const Token& token);
    const std::vector<Token>& tokens() const { return content_tokens; }

private:
    std::vector<Token> content_tokens;
};

class PropertyNode final : public ASTNode {
public:
    PropertyNode(const Token& name_token, std::vector<Token> value_tokens);

    const std::string& name() const { return property_name; }
    const std::vector<Token>& value() const { return value_tokens; }

private:
    std::string property_name;
    std::vector<Token> value_tokens;
};

class VariableNode final : public ASTNode {
public:
    VariableNode(const Token& name_token, std::vector<Token> value_tokens);

    const std::string& name() const { return variable_name; }
    const std::vector<Token>& value() const { return value_tokens; }

private:
    std::string variable_name;
    std::vector<Token> value_tokens;
};

class TagNode final : public ASTNode {
public:
    explicit TagNode(const Token& name_token);

    const std::string& name() const { return tag_name; }

    void set_content(std::unique_ptr<ContentNode> content);
    ContentNode* content() const { return content_node; }

    void add_property(std::unique_ptr<PropertyNode> property);
    void add_variable(std::unique_ptr<VariableNode> variable);
    void add_child(std::unique_ptr<TagNode> child);

    const std::vector<std::unique_ptr<ASTNode>>& children() const { return child_nodes; }

private:
    std::string tag_name;
    ContentNode* content_node = nullptr;
    std::vector<std::unique_ptr<ASTNode>> child_nodes;
};

class RootNode final : public ASTNode {
public:
    explicit RootNode(const Token& root_token);

    void add_property(std::unique_ptr<PropertyNode> property);
    void add_variable(std::unique_ptr<VariableNode> variable);
    void add_tag(std::unique_ptr<TagNode> tag);

    const std::vector<std::unique_ptr<ASTNode>>& children() const { return child_nodes; }

private:
    std::vector<std::unique_ptr<ASTNode>> child_nodes;
};

void print_ast(const RootNode& root);

#endif