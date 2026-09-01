#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <cassert>
#include <memory>
#include <string>

static std::unique_ptr<RootNode> parse(const std::string& source)
{
    Lexer lexer(source, "test.csam");
    const auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

int main()
{
    {
        auto root = parse(":root { div { color: red; } }");
        assert(root != nullptr);
        assert(root->children().size() == 1);
        assert(root->children()[0]->type() == ASTNodeType::Tag);
    }

    {
        auto root = parse(":root { div <hello> }");
        assert(root->children().size() == 1);
        auto* tag = dynamic_cast<TagNode*>(root->children()[0].get());
        assert(tag != nullptr);
        assert(tag->content() != nullptr);
        assert(tag->content()->tokens().size() == 1);
        assert(tag->content()->tokens()[0].value == "hello");
    }

    {
        auto root = parse(":root { var primary = red; }");
        assert(root->children().size() == 1);
        assert(root->children()[0]->type() == ASTNodeType::Variable);
    }

    {
        auto root = parse(":root { div {} span <hello> }");
        assert(root->children().size() == 2);
        assert(root->children()[0]->type() == ASTNodeType::Tag);
        assert(root->children()[1]->type() == ASTNodeType::Tag);
    }

    return 0;
}
