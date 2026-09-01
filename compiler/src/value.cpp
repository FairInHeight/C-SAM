#include "value.hpp"

#include <utility>

NumberValueNode::NumberValueNode(const Token& token, std::string representation)
    : ValueNode(ASTNodeType::NumberValue, token),
      representation(std::move(representation))
{
}

DimensionValueNode::DimensionValueNode(
    const Token& token,
    std::string number,
    std::string unit)
    : ValueNode(ASTNodeType::DimensionValue, token),
      numeric_value(std::move(number)),
      unit_value(std::move(unit))
{
}

PercentageValueNode::PercentageValueNode(const Token& token, std::string number)
    : ValueNode(ASTNodeType::PercentageValue, token),
      numeric_value(std::move(number))
{
}

StringValueNode::StringValueNode(const Token& token, std::string value)
    : ValueNode(ASTNodeType::StringValue, token),
      string_value(std::move(value))
{
}

RawValueNode::RawValueNode(const Token& token, std::string value)
    : ValueNode(ASTNodeType::RawValue, token),
      raw_value(std::move(value))
{
}
