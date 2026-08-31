#include "ast.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string tokens_to_string(const std::vector<Token>& tokens)
{
    std::ostringstream output;

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (i != 0) {
            output << ' ';
        }

        output << tokens[i].value;
    }

    return output.str();
}

void print_content(const ContentNode& content, std::size_t depth)
{
    std::cout << std::string(depth * 4, ' ') << "Content: "
              << tokens_to_string(content.tokens()) << '\n';
}

void print_variable(const VariableNode& variable, std::size_t depth)
{
    std::cout << std::string(depth * 4, ' ') << "Variable: "
              << variable.name() << " = "
              << tokens_to_string(variable.value()) << '\n';
}

void print_property(const PropertyNode& property, std::size_t depth)
{
    std::cout << std::string(depth * 4, ' ') << "Property: "
              << property.name() << " = "
              << tokens_to_string(property.value()) << '\n';
}

void print_tag(const TagNode& tag, std::size_t depth)
{
    std::cout << std::string(depth * 4, ' ') << "Tag: " << tag.name() << '\n';

    if (tag.content() != nullptr) {
        print_content(*tag.content(), depth + 1);
    }

    for (const auto& variable : tag.variables()) {
        print_variable(*variable, depth + 1);
    }

    for (const auto& property : tag.properties()) {
        print_property(*property, depth + 1);
    }

    for (const auto& child : tag.children()) {
        print_tag(*child, depth + 1);
    }
}

}

ASTNode::ASTNode(ASTNodeType type, const Token& token)
    : node_type(type),
      source_filepath(token.filepath),
      source_line(token.line),
      source_column(token.column)
{
}

ContentNode::ContentNode(const Token& token)
    : ASTNode(ASTNodeType::Content, token)
{
}

void ContentNode::add_token(const Token& token)
{
    content_tokens.push_back(token);
}

PropertyNode::PropertyNode(const Token& name_token, std::vector<Token> value_tokens)
    : ASTNode(ASTNodeType::Property, name_token),
      property_name(name_token.value),
      value_tokens(std::move(value_tokens))
{
}

VariableNode::VariableNode(const Token& name_token, std::vector<Token> value_tokens)
    : ASTNode(ASTNodeType::Variable, name_token),
      variable_name(name_token.value),
      value_tokens(std::move(value_tokens))
{
}

TagNode::TagNode(const Token& name_token)
    : ASTNode(ASTNodeType::Tag, name_token),
      tag_name(name_token.value)
{
}

void TagNode::set_content(std::unique_ptr<ContentNode> content)
{
    content_node = std::move(content);
}

void TagNode::add_property(std::unique_ptr<PropertyNode> property)
{
    property_nodes.push_back(std::move(property));
}

void TagNode::add_variable(std::unique_ptr<VariableNode> variable)
{
    variable_nodes.push_back(std::move(variable));
}

void TagNode::add_child(std::unique_ptr<TagNode> child)
{
    child_nodes.push_back(std::move(child));
}

RootNode::RootNode(const Token& root_token)
    : ASTNode(ASTNodeType::Root, root_token)
{
}

void RootNode::add_property(std::unique_ptr<PropertyNode> property)
{
    property_nodes.push_back(std::move(property));
}

void RootNode::add_variable(std::unique_ptr<VariableNode> variable)
{
    variable_nodes.push_back(std::move(variable));
}

void RootNode::add_tag(std::unique_ptr<TagNode> tag)
{
    tag_nodes.push_back(std::move(tag));
}

void print_ast(const RootNode& root)
{
    std::cout << "AST:\n";

    for (const auto& variable : root.variables()) {
        print_variable(*variable, 1);
    }

    for (const auto& property : root.properties()) {
        print_property(*property, 1);
    }

    for (const auto& tag : root.tags()) {
        print_tag(*tag, 1);
    }
}
