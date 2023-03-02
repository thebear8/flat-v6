#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../data/operator.hpp"
#include "ir_node.hpp"

struct IRFunction;

struct IRExpression : public IRNode
{
    IMPLEMENT_ACCEPT()

    METADATA_PROP(type, IRType*, getType, setType)
};

struct IRIntegerExpression : public IRExpression
{
    bool isSigned;
    size_t width, radix;
    std::string value;

    IRIntegerExpression(
        bool isSigned, size_t width, size_t radix, std::string const& value
    )
        : isSigned(isSigned), width(width), radix(radix), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRBoolExpression : public IRExpression
{
    bool value;

    IRBoolExpression(bool value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRCharExpression : public IRExpression
{
    uint32_t value;

    IRCharExpression(uint32_t value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRStringExpression : public IRExpression
{
    std::vector<uint8_t> value;

    IRStringExpression(std::vector<uint8_t> const& value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRIdentifierExpression : public IRExpression
{
    std::string value;
    std::vector<IRType*> typeArgs;

    IRIdentifierExpression(
        std::string const& value, std::vector<IRType*> const& typeArgs
    )
        : value(value), typeArgs(typeArgs)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRStructExpression : public IRExpression
{
    std::string structName;
    std::vector<IRType*> typeArgs;
    std::unordered_map<std::string, IRExpression*> fields;

    IRStructExpression(
        std::string const& structName,
        std::vector<IRType*> const& typeArgs,
        std::unordered_map<std::string, IRExpression*> const& fields
    )
        : structName(structName), typeArgs(typeArgs), fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRUnaryExpression : public IRExpression
{
    UnaryOperator operation;
    IRExpression* expression;

    IRUnaryExpression(UnaryOperator operation, IRExpression* expression)
        : operation(operation), expression(expression)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(target, IRFunction*, getTarget, setTarget)
    METADATA_PROP(
        targetTypeArgs,
        std::vector<IRType*>,
        getTargetTypeArgs,
        setTargetTypeArgs
    )
};

struct IRBinaryExpression : public IRExpression
{
    BinaryOperator operation;
    IRExpression *left, *right;

    IRBinaryExpression(
        BinaryOperator operation, IRExpression* left, IRExpression* right
    )
        : operation(operation), left(left), right(right)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(target, IRFunction*, getTarget, setTarget)
    METADATA_PROP(
        targetTypeArgs,
        std::vector<IRType*>,
        getTargetTypeArgs,
        setTargetTypeArgs
    )
};

struct IRCallExpression : public IRExpression
{
    IRExpression* expression;
    std::vector<IRExpression*> args;

    IRCallExpression(
        IRExpression* expression, std::vector<IRExpression*> const& args
    )
        : expression(expression), args(args)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(target, IRFunction*, getTarget, setTarget)
    METADATA_PROP(
        targetTypeArgs,
        std::vector<IRType*>,
        getTargetTypeArgs,
        setTargetTypeArgs
    )
};

struct IRIndexExpression : public IRExpression
{
    IRExpression* expression;
    std::vector<IRExpression*> args;

    IRIndexExpression(
        IRExpression* expression, std::vector<IRExpression*> const& args
    )
        : expression(expression), args(args)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(target, IRFunction*, getTarget, setTarget)
    METADATA_PROP(
        targetTypeArgs,
        std::vector<IRType*>,
        getTargetTypeArgs,
        setTargetTypeArgs
    )
};

struct IRFieldExpression : public IRExpression
{
    IRExpression* expression;
    std::string fieldName;

    IRFieldExpression(IRExpression* expression, std::string const& fieldName)
        : expression(expression), fieldName(fieldName)
    {
    }

    IMPLEMENT_ACCEPT()
};