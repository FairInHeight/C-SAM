#include "ast.hpp"
#include "parser.hpp"
#include "lexer.hpp"

#include <cassert>
#include <string>

static Parser parse(const std::string& source)
{
    Lexer lexer(source, "test.csam");
    return Parser(lexer.tokenize());
}

int main()
{
    {
        auto parser = parse("div { color: red; }");
        auto root = parser.parse();
        assert(root != nullptr);
        assert(root->children.size() == 1);
        assert(root->children[0]->type == ASTNodeType::Tag);
    }

    {
        auto parser = parse("div <hello>");
        auto root = parser.parse();
        assert(root->children.size() == 1);
        auto* tag = dynamic_cast<TagNode*>(root->children[0].get());
        assert(tag != nullptr);
        assert(tag->children.size() == 1);
        assert(tag->children[0]->type == ASTNodeType::Content);
    }

    {
        auto parser = parse("var: primary = red;");
        auto root = parser.parse();
        assert(root->children.size() == 1);
        assert(root->children[0]->type == ASTNodeType::Variable);
    }

    {
        auto parser = parse("div {}\nspan <>");
        auto root = parser.parse();
        assert(root->children.size() == 2);
        assert(root->children[0]->type == ASTNodeType::Tag);
        assert(root->children[1]->type == ASTNodeType::Tag);
    }

    return 0;
}
