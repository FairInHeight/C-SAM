#include "ast.hpp"

#include <iostream>
#include <sstream>
#include <utility>

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

void print_node(const ASTNode& node, std::size_t depth)
{
    const std::string indent(depth * 4, ' ');

    switch (node.type()) {
        case ASTNodeType::Root: {
            const auto& root = static_cast<const RootNode&>(node);
            for (const auto& child : root.children()) {
                print_node(*child, depth);
            }
            break;
        }

        case ASTNodeType::Tag: {
            const auto& tag = static_cast<const TagNode&>(node);
            std::cout << indent << "Tag: " << tag.name() << '\n';
            for (const auto& child : tag.children()) {
                print_node(*child, depth + 1);
            }
            break;
        }

        case ASTNodeType::Content: {
            const auto& content = static_cast<const ContentNode&>(node);
            std::cout << indent << "Content: "
                      << tokens_to_string(content.tokens()) << '\n';
            break;
        }

        case ASTNodeType::Property: {
            const auto& property = static_cast<const PropertyNode&>(node);
            std::cout << indent << "Property: "
                      << property.name() << " = "
                      << tokens_to_string(property.value()) << '\n';
            break;
        }

        case ASTNodeType::Variable: {
            const auto& variable = static_cast<const VariableNode&>(node);
            std::cout << indent << "Variable: "
                      << variable.name() << " = "
                      << tokens_to_string(variable.value()) << '\n';
            break;
        }
    }
}

}

ASTNode::ASTNode(ASTNodeType type, const Token& token)
    : node_type(type),
      source_location(token.location)
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
    content_node = content.get();
    child_nodes.push_back(std::move(content));
}

void TagNode::add_property(std::unique_ptr<PropertyNode> property)
{
    child_nodes.push_back(std::move(property));
}

void TagNode::add_variable(std::unique_ptr<VariableNode> variable)
{
    child_nodes.push_back(std::move(variable));
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
    child_nodes.push_back(std::move(property));
}

void RootNode::add_variable(std::unique_ptr<VariableNode> variable)
{
    child_nodes.push_back(std::move(variable));
}

void RootNode::add_tag(std::unique_ptr<TagNode> tag)
{
    child_nodes.push_back(std::move(tag));
}

void print_ast(const RootNode& root)
{
    std::cout << "AST:\n";
    print_node(root, 1);
}
