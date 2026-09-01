#include "lexer.hpp"
#include "value_parser.hpp"
#include "value.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

static std::vector<std::unique_ptr<ValueNode>> parse_value(const std::string& source)
{
    Lexer lexer(source, "test.csam");
    const auto tokens = lexer.tokenize();
    std::size_t current = 0;
    ValueParser parser(tokens, current);
    return parser.parse_value();
}

static void assert_parse_error(const std::string& source)
{
    bool failed = false;

    try {
        (void)parse_value(source);
    } catch (const std::runtime_error&) {
        failed = true;
    }

    assert(failed);
}

int main()
{
    {
        auto values = parse_value("red;");
        assert(values.size() == 1);
        assert(values[0]->type() == ASTNodeType::RawValue);
        assert(static_cast<const RawValueNode&>(*values[0]).value() == "red");
    }

    {
        auto values = parse_value("100px 50% 0.5;");
        assert(values.size() == 3);

        assert(values[0]->type() == ASTNodeType::DimensionValue);
        assert(static_cast<const DimensionValueNode&>(*values[0]).number() == "100");
        assert(static_cast<const DimensionValueNode&>(*values[0]).unit() == "px");

        assert(values[1]->type() == ASTNodeType::PercentageValue);
        assert(static_cast<const PercentageValueNode&>(*values[1]).number() == "50");

        assert(values[2]->type() == ASTNodeType::NumberValue);
        assert(static_cast<const NumberValueNode&>(*values[2]).value() == "0.5");
    }

    {
        auto values = parse_value("\"hello\";");
        assert(values.size() == 1);
        assert(values[0]->type() == ASTNodeType::StringValue);
        assert(static_cast<const StringValueNode&>(*values[0]).value() == "\"hello\"");
    }

    {
        auto values = parse_value("rgb(255, 0, 128);");
        assert(values.size() == 1);
        assert(values[0]->type() == ASTNodeType::FunctionValue);

        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.name() == "rgb");
        assert(function.arguments().size() == 3);
        assert(function.arguments()[0].size() == 1);
        assert(function.arguments()[1].size() == 1);
        assert(function.arguments()[2].size() == 1);
        assert(static_cast<const NumberValueNode&>(*function.arguments()[0][0]).value() == "255");
        assert(static_cast<const NumberValueNode&>(*function.arguments()[1][0]).value() == "0");
        assert(static_cast<const NumberValueNode&>(*function.arguments()[2][0]).value() == "128");
    }

    {
        auto values = parse_value("foo(10px, \"hello\", red);");
        assert(values.size() == 1);
        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.name() == "foo");
        assert(function.arguments().size() == 3);
        assert(function.arguments()[0].size() == 1);
        assert(function.arguments()[1].size() == 1);
        assert(function.arguments()[2].size() == 1);
        assert(function.arguments()[0][0]->type() == ASTNodeType::DimensionValue);
        assert(function.arguments()[1][0]->type() == ASTNodeType::StringValue);
        assert(function.arguments()[2][0]->type() == ASTNodeType::RawValue);
    }

    {
        auto values = parse_value("calc(100% - 20px);");
        assert(values.size() == 1);
        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.name() == "calc");
        assert(function.arguments().size() == 1);
        assert(function.arguments()[0].size() == 3);
        assert(function.arguments()[0][0]->type() == ASTNodeType::PercentageValue);
        assert(function.arguments()[0][1]->type() == ASTNodeType::RawValue);
        assert(function.arguments()[0][2]->type() == ASTNodeType::DimensionValue);
    }

    {
        auto values = parse_value("foo();");
        assert(values.size() == 1);
        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.name() == "foo");
        assert(function.arguments().empty());
    }

    {
        auto values = parse_value("foo(bar(10px), baz(50%));");
        assert(values.size() == 1);
        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.arguments().size() == 2);
        assert(function.arguments()[0].size() == 1);
        assert(function.arguments()[1].size() == 1);

        const auto& nested_bar = static_cast<const FunctionValueNode&>(*function.arguments()[0][0]);
        const auto& nested_baz = static_cast<const FunctionValueNode&>(*function.arguments()[1][0]);
        assert(nested_bar.name() == "bar");
        assert(nested_baz.name() == "baz");
        assert(nested_bar.arguments().size() == 1);
        assert(nested_baz.arguments().size() == 1);
    }

    {
        auto values = parse_value("10 px;");
        assert(values.size() == 2);
        assert(values[0]->type() == ASTNodeType::NumberValue);
        assert(values[1]->type() == ASTNodeType::RawValue);
    }

    // Boundary and whitespace cases.
    {
        auto values = parse_value("-42 -3.5 1e3 -2.5e-2 0;");
        assert(values.size() == 5);
        assert(static_cast<const NumberValueNode&>(*values[0]).value() == "-42");
        assert(static_cast<const NumberValueNode&>(*values[1]).value() == "-3.5");
        assert(static_cast<const NumberValueNode&>(*values[2]).value() == "1e3");
        assert(static_cast<const NumberValueNode&>(*values[3]).value() == "-2.5e-2");
        assert(static_cast<const NumberValueNode&>(*values[4]).value() == "0");
    }

    {
        auto values = parse_value("-10px -25% 0px 0%;");
        assert(values.size() == 4);
        assert(static_cast<const DimensionValueNode&>(*values[0]).number() == "-10");
        assert(static_cast<const PercentageValueNode&>(*values[1]).number() == "-25");
        assert(static_cast<const DimensionValueNode&>(*values[2]).number() == "0");
        assert(static_cast<const PercentageValueNode&>(*values[3]).number() == "0");
    }

    {
        auto values = parse_value("foo( 10px , 20% , red );");
        assert(values.size() == 1);
        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.arguments().size() == 3);
        assert(function.arguments()[0][0]->type() == ASTNodeType::DimensionValue);
        assert(function.arguments()[1][0]->type() == ASTNodeType::PercentageValue);
        assert(function.arguments()[2][0]->type() == ASTNodeType::RawValue);
    }

    {
        auto values = parse_value("calc(100% - 20px + 5px);");
        assert(values.size() == 1);
        const auto& function = static_cast<const FunctionValueNode&>(*values[0]);
        assert(function.arguments().size() == 1);
        assert(function.arguments()[0].size() == 5);
        assert(function.arguments()[0][0]->type() == ASTNodeType::PercentageValue);
        assert(function.arguments()[0][1]->type() == ASTNodeType::RawValue);
        assert(function.arguments()[0][2]->type() == ASTNodeType::DimensionValue);
        assert(function.arguments()[0][3]->type() == ASTNodeType::RawValue);
        assert(function.arguments()[0][4]->type() == ASTNodeType::DimensionValue);
    }

    // Invalid function arguments must be rejected.
    assert_parse_error("foo(, 10px);");
    assert_parse_error("foo(10px,);");
    assert_parse_error("foo(10px;");
    assert_parse_error("foo(10px, 20px;");

    // A value parser must reject an empty value rather than producing an empty AST.
    assert_parse_error(";");

    return 0;
}
