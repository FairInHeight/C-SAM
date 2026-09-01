#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

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
        auto* tag = dynamic_cast<TagNode*>(root->children()[0].get());
        assert(tag != nullptr);
        assert(tag->children().size() == 1);

        auto* property = dynamic_cast<PropertyNode*>(tag->children()[0].get());
        assert(property != nullptr);
        assert(property->value().size() == 1);
        assert(property->value()[0]->type() == ASTNodeType::RawValue);
        assert(static_cast<const RawValueNode&>(*property->value()[0]).value() == "red");
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
        auto* variable = dynamic_cast<VariableNode*>(root->children()[0].get());
        assert(variable != nullptr);
        assert(variable->value().size() == 1);
        assert(variable->value()[0]->type() == ASTNodeType::RawValue);
        assert(static_cast<const RawValueNode&>(*variable->value()[0]).value() == "red");
    }

    {
        auto root = parse(":root { div {} span <hello> }");
        assert(root->children().size() == 2);
        assert(root->children()[0]->type() == ASTNodeType::Tag);
        assert(root->children()[1]->type() == ASTNodeType::Tag);
    }

    {
        auto root = parse(
            ":root { div { width: 100px; opacity: 0.5; ratio: 1e3; "
            "percent: 50%; text: \"hello\"; } }");
        auto* tag = dynamic_cast<TagNode*>(root->children()[0].get());
        assert(tag != nullptr);
        assert(tag->children().size() == 5);

        auto* width = dynamic_cast<PropertyNode*>(tag->children()[0].get());
        assert(width != nullptr);
        assert(width->value().size() == 1);
        assert(width->value()[0]->type() == ASTNodeType::DimensionValue);
        assert(static_cast<const DimensionValueNode&>(*width->value()[0]).number() == "100");
        assert(static_cast<const DimensionValueNode&>(*width->value()[0]).unit() == "px");

        auto* opacity = dynamic_cast<PropertyNode*>(tag->children()[1].get());
        assert(opacity != nullptr);
        assert(opacity->value().size() == 1);
        assert(opacity->value()[0]->type() == ASTNodeType::NumberValue);
        assert(static_cast<const NumberValueNode&>(*opacity->value()[0]).value() == "0.5");

        auto* ratio = dynamic_cast<PropertyNode*>(tag->children()[2].get());
        assert(ratio != nullptr);
        assert(ratio->value().size() == 1);
        assert(ratio->value()[0]->type() == ASTNodeType::NumberValue);
        assert(static_cast<const NumberValueNode&>(*ratio->value()[0]).value() == "1e3");

        auto* percent = dynamic_cast<PropertyNode*>(tag->children()[3].get());
        assert(percent != nullptr);
        assert(percent->value().size() == 1);
        assert(percent->value()[0]->type() == ASTNodeType::PercentageValue);
        assert(static_cast<const PercentageValueNode&>(*percent->value()[0]).number() == "50");

        auto* text = dynamic_cast<PropertyNode*>(tag->children()[4].get());
        assert(text != nullptr);
        assert(text->value().size() == 1);
        assert(text->value()[0]->type() == ASTNodeType::StringValue);
        assert(static_cast<const StringValueNode&>(*text->value()[0]).value() == "\"hello\"");
    }

    {
        // Whitespace prevents the number and identifier from becoming a
        // dimension. The source locations make that distinction explicit.
        auto root = parse(":root { div { width: 10 px; } }");
        auto* tag = dynamic_cast<TagNode*>(root->children()[0].get());
        assert(tag != nullptr);
        auto* property = dynamic_cast<PropertyNode*>(tag->children()[0].get());
        assert(property != nullptr);
        assert(property->value().size() == 2);
        assert(property->value()[0]->type() == ASTNodeType::NumberValue);
        assert(property->value()[1]->type() == ASTNodeType::RawValue);
        assert(static_cast<const NumberValueNode&>(*property->value()[0]).value() == "10");
        assert(static_cast<const RawValueNode&>(*property->value()[1]).value() == "px");
    }

    return 0;
}
