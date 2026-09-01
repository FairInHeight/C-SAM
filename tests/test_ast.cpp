#include "ast.hpp"
#include "token.hpp"

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

        root->add_property(std::make_unique<PropertyNode>(
            token(TokenType::Identifier, "color"),
            std::vector<Token>{token(TokenType::Identifier, "red")}));

        root->add_variable(std::make_unique<VariableNode>(
            token(TokenType::Identifier, "primary"),
            std::vector<Token>{token(TokenType::Identifier, "red")}));

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
            token(TokenType::LeftAngle, "<"));
        assert(content->tokens().empty());

        content->add_token(token(TokenType::Identifier, "hello"));
        content->add_token(token(TokenType::Identifier, "world"));
        tag->set_content(std::move(content));

        assert(tag->content() != nullptr);
        assert(tag->content()->tokens().size() == 2);
        assert(tag->content()->tokens()[0].value == "hello");
        assert(tag->content()->tokens()[1].value == "world");

        tag->add_property(std::make_unique<PropertyNode>(
            token(TokenType::Identifier, "color"),
            std::vector<Token>{token(TokenType::Identifier, "red")}));

        tag->add_variable(std::make_unique<VariableNode>(
            token(TokenType::Identifier, "primary"),
            std::vector<Token>{token(TokenType::Identifier, "red")}));

        tag->add_child(std::make_unique<TagNode>(
            token(TokenType::Identifier, "span")));

        assert(tag->children().size() == 4);
        assert(tag->children()[0]->type() == ASTNodeType::Content);
        assert(tag->children()[1]->type() == ASTNodeType::Property);
        assert(tag->children()[2]->type() == ASTNodeType::Variable);
        assert(tag->children()[3]->type() == ASTNodeType::Tag);
    }

    {
        PropertyNode property(
            token(TokenType::Identifier, "margin"),
            std::vector<Token>{
                token(TokenType::Number, "10"),
                token(TokenType::Identifier, "px"),
                token(TokenType::Identifier, "auto")});

        assert(property.type() == ASTNodeType::Property);
        assert(property.name() == "margin");
        assert(property.value().size() == 3);
        assert(property.value()[0].value == "10");
        assert(property.value()[1].value == "px");
        assert(property.value()[2].value == "auto");
    }

    return 0;
}
