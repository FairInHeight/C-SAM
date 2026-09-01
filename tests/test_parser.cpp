#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

static std::unique_ptr<RootNode> parse(const std::string& source)
{
    Lexer lexer(source, "test.csam");
    const auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

static void expect_parse_error(const std::string& source, const std::string& message)
{
    bool threw = false;

    try {
        (void)parse(source);
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string text = error.what();
        if (text.find(message) == std::string::npos) {
            std::cerr << "Parser negative test diagnostic mismatch\n"
                      << "  Source:   " << source << '\n'
                      << "  Expected: " << message << '\n'
                      << "  Actual:   " << text << '\n';
            throw std::runtime_error("Parser negative test diagnostic mismatch");
        }
    }

    if (!threw) {
        std::cerr << "Parser negative test expected an error but parsing succeeded\n"
                  << "  Source:   " << source << '\n'
                  << "  Expected: " << message << '\n';
        throw std::runtime_error("Parser negative test expected an error");
    }
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

    // Parser/value-parser integration: functions must survive structural parsing
    // and appear as FunctionValueNodes in the property's AST.
    {
        auto root = parse(
            ":root { div { "
            "color: rgb(255, 0, 0); "
            "width: calc(100% - 20px); "
            "transform: scale(2, translate(10px, 20px)); "
            "} }");

        auto* tag = dynamic_cast<TagNode*>(root->children()[0].get());
        assert(tag != nullptr);
        assert(tag->children().size() == 3);

        auto* color = dynamic_cast<PropertyNode*>(tag->children()[0].get());
        assert(color != nullptr);
        assert(color->value().size() == 1);
        assert(color->value()[0]->type() == ASTNodeType::FunctionValue);
        const auto& rgb = static_cast<const FunctionValueNode&>(*color->value()[0]);
        assert(rgb.name() == "rgb");
        assert(rgb.arguments().size() == 3);
        assert(rgb.arguments()[0].size() == 1);
        assert(rgb.arguments()[1].size() == 1);
        assert(rgb.arguments()[2].size() == 1);
        assert(static_cast<const NumberValueNode&>(*rgb.arguments()[0][0]).value() == "255");
        assert(static_cast<const NumberValueNode&>(*rgb.arguments()[1][0]).value() == "0");
        assert(static_cast<const NumberValueNode&>(*rgb.arguments()[2][0]).value() == "0");

        auto* width = dynamic_cast<PropertyNode*>(tag->children()[1].get());
        assert(width != nullptr);
        assert(width->value().size() == 1);
        const auto& calc = static_cast<const FunctionValueNode&>(*width->value()[0]);
        assert(calc.name() == "calc");
        assert(calc.arguments().size() == 1);
        assert(calc.arguments()[0].size() == 3);
        assert(calc.arguments()[0][0]->type() == ASTNodeType::PercentageValue);
        assert(calc.arguments()[0][1]->type() == ASTNodeType::RawValue);
        assert(static_cast<const RawValueNode&>(*calc.arguments()[0][1]).value() == "-");
        assert(calc.arguments()[0][2]->type() == ASTNodeType::DimensionValue);

        auto* transform = dynamic_cast<PropertyNode*>(tag->children()[2].get());
        assert(transform != nullptr);
        assert(transform->value().size() == 1);
        const auto& scale = static_cast<const FunctionValueNode&>(*transform->value()[0]);
        assert(scale.name() == "scale");
        assert(scale.arguments().size() == 2);
        assert(scale.arguments()[0].size() == 1);
        assert(static_cast<const NumberValueNode&>(*scale.arguments()[0][0]).value() == "2");
        assert(scale.arguments()[1].size() == 1);
        assert(scale.arguments()[1][0]->type() == ASTNodeType::FunctionValue);

        const auto& translate = static_cast<const FunctionValueNode&>(*scale.arguments()[1][0]);
        assert(translate.name() == "translate");
        assert(translate.arguments().size() == 2);
        assert(translate.arguments()[0].size() == 1);
        assert(translate.arguments()[1].size() == 1);
        assert(static_cast<const DimensionValueNode&>(*translate.arguments()[0][0]).unit() == "px");
        assert(static_cast<const DimensionValueNode&>(*translate.arguments()[1][0]).unit() == "px");
    }

    // Error-path coverage: malformed structure and declarations must fail
    // through the parser's normal runtime_error diagnostic path.
    expect_parse_error("root { div {} }", "Expected ':' before root name");
    expect_parse_error(":notroot { div {} }", "Expected root name 'root'");
    expect_parse_error(":root { div {", "unmatched opening delimiter");
    expect_parse_error(":root { div {} } }", "Unmatched '}'");
    expect_parse_error(":root { div { color red; } }", "Expected ':' after property name");
    expect_parse_error(":root { div { color: red } }", "Expected ';' after property value");
    expect_parse_error(":root { var primary; }", "Expected '=' after variable name");
    expect_parse_error(":root { div { color: ; } }", "Expected value");
    expect_parse_error(":root { div { color: rgb(255, 0, 0; } }", "unmatched opening delimiter");
    expect_parse_error(":root { div { color: rgb(255, ); } }", "Expected function argument after ','");
    expect_parse_error(":root { div { color: rgb(, 255); } }", "Expected function argument");
    expect_parse_error(":root { div { color: rgb(255, 0, ); } }", "Expected function argument after ','");
    expect_parse_error(":root { div { color: rgb(255, 0, 0); } } trailing", "Unexpected token after root block");

    return 0;
}
