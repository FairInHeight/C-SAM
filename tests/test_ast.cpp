#include "ast.hpp"
#include "token.hpp"
#include "value.hpp"

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

static Token token(TokenType type, const std::string& value = "")
{
    return Token{type, value, {std::filesystem::path("test.csam"), 1, 1}};
}

int main()
{
    {
        auto root = std::make_unique<RootNode>(token(TokenType::Identifier, ":root"));
        assert(root->type() == ASTNodeType::Root);
        assert(root->location().line == 1);
        assert(root->location().column == 1);
        assert(root->children().empty());

        std::vector<std::unique_ptr<ValueNode>> property_values;
        property_values.push_back(std::make_unique<RawValueNode>(
            token(TokenType::Identifier, "red"), "red"));
        root->add_property(std::make_unique<PropertyNode>(
            token(TokenType::Identifier, "color"),
            std::move(property_values)));

        std::vector<std::unique_ptr<ValueNode>> variable_values;
        variable_values.push_back(std::make_unique<RawValueNode>(
            token(TokenType::Identifier, "red"), "red"));
        root->add_variable(std::make_unique<VariableNode>(
            token(TokenType::Identifier, "primary"),
            std::move(variable_values)));

        root->add_tag(std::make_unique<TagNode>(
            token(TokenType::Identifier, "div")));

        assert(root->children().size() == 3);
        assert(root->children()[0]->type() == ASTNodeType::Property);
        assert(root->children()[1]->type() == ASTNodeType::Variable);
        assert(root->children()[2]->type() == ASTNodeType::Tag);
    }

    {
        auto tag = std::make_unique<TagNode>(token(TokenType::Identifier, "div"));
        assert(tag->type() == ASTNodeType::Tag);
        assert(tag->name() == "div");
        assert(tag->content() == nullptr);
        assert(tag->children().empty());

        auto content = std::make_unique<ContentNode>(
            token(TokenType::LessThan, "<"));
        assert(content->tokens().empty());

        content->add_token(token(TokenType::Identifier, "hello"));
        content->add_token(token(TokenType::Identifier, "world"));
        tag->set_content(std::move(content));

        assert(tag->content() != nullptr);
        assert(tag->content()->tokens().size() == 2);
        assert(tag->content()->tokens()[0].value == "hello");
        assert(tag->content()->tokens()[1].value == "world");

        std::vector<std::unique_ptr<ValueNode>> property_values;
        property_values.push_back(std::make_unique<RawValueNode>(
            token(TokenType::Identifier, "red"), "red"));
        tag->add_property(std::make_unique<PropertyNode>(
            token(TokenType::Identifier, "color"),
            std::move(property_values)));

        std::vector<std::unique_ptr<ValueNode>> variable_values;
        variable_values.push_back(std::make_unique<RawValueNode>(
            token(TokenType::Identifier, "red"), "red"));
        tag->add_variable(std::make_unique<VariableNode>(
            token(TokenType::Identifier, "primary"),
            std::move(variable_values)));

        tag->add_child(std::make_unique<TagNode>(
            token(TokenType::Identifier, "span")));

        assert(tag->children().size() == 4);
        assert(tag->children()[0]->type() == ASTNodeType::Content);
        assert(tag->children()[1]->type() == ASTNodeType::Property);
        assert(tag->children()[2]->type() == ASTNodeType::Variable);
        assert(tag->children()[3]->type() == ASTNodeType::Tag);
    }

    {
        std::vector<std::unique_ptr<ValueNode>> values;
        values.push_back(std::make_unique<DimensionValueNode>(
            token(TokenType::Number, "10"), "10", "px"));
        values.push_back(std::make_unique<RawValueNode>(
            token(TokenType::Identifier, "auto"), "auto"));

        PropertyNode property(
            token(TokenType::Identifier, "margin"),
            std::move(values));

        assert(property.type() == ASTNodeType::Property);
        assert(property.name() == "margin");
        assert(property.value().size() == 2);
        assert(property.value()[0]->type() == ASTNodeType::DimensionValue);
        assert(static_cast<const DimensionValueNode&>(*property.value()[0]).number() == "10");
        assert(static_cast<const DimensionValueNode&>(*property.value()[0]).unit() == "px");
        assert(property.value()[1]->type() == ASTNodeType::RawValue);
        assert(static_cast<const RawValueNode&>(*property.value()[1]).value() == "auto");
    }

    {
        const Token number = token(TokenType::Number, "10.5");
        NumberValueNode value(number, "10.5");
        assert(value.type() == ASTNodeType::NumberValue);
        assert(value.value() == "10.5");
        assert(value.location().line == 1);
    }

    {
        const Token number = token(TokenType::Number, "10");
        DimensionValueNode value(number, "10", "px");
        assert(value.type() == ASTNodeType::DimensionValue);
        assert(value.number() == "10");
        assert(value.unit() == "px");
    }

    {
        const Token number = token(TokenType::Number, "50");
        PercentageValueNode value(number, "50");
        assert(value.type() == ASTNodeType::PercentageValue);
        assert(value.number() == "50");
    }

    {
        const Token string = token(TokenType::String, "\"hello\"");
        StringValueNode value(string, "\"hello\"");
        assert(value.type() == ASTNodeType::StringValue);
        assert(value.value() == "\"hello\"");
    }

    {
        const Token raw = token(TokenType::Identifier, "auto");
        RawValueNode value(raw, "auto");
        assert(value.type() == ASTNodeType::RawValue);
        assert(value.value() == "auto");
    }

    return 0;
}
