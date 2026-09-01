#ifndef CSAM_VALUE_HPP
#define CSAM_VALUE_HPP

#include "ast.hpp"

#include <string>

class ValueNode : public ASTNode {
public:
    using ASTNode::ASTNode;
    ~ValueNode() override = default;
};

class NumberValueNode final : public ValueNode {
public:
    NumberValueNode(const Token& token, std::string representation);

    const std::string& value() const { return representation; }

private:
    std::string representation;
};

class DimensionValueNode final : public ValueNode {
public:
    DimensionValueNode(const Token& token, std::string number, std::string unit);

    const std::string& number() const { return numeric_value; }
    const std::string& unit() const { return unit_value; }

private:
    std::string numeric_value;
    std::string unit_value;
};

class PercentageValueNode final : public ValueNode {
public:
    PercentageValueNode(const Token& token, std::string number);

    const std::string& number() const { return numeric_value; }

private:
    std::string numeric_value;
};

class StringValueNode final : public ValueNode {
public:
    StringValueNode(const Token& token, std::string value);

    const std::string& value() const { return string_value; }

private:
    std::string string_value;
};

// Temporary lossless representation for value syntax that does not yet have
// a dedicated semantic node. This lets the parser become semantic without
// throwing away CSS constructs we have not modeled yet.
class RawValueNode final : public ValueNode {
public:
    RawValueNode(const Token& token, std::string value);

    const std::string& value() const { return raw_value; }

private:
    std::string raw_value;
};

#endif
