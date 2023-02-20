#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../data/operator.hpp"
#include "ir_node.hpp"

struct IRExpression : public IRNode
{
    IMPLEMENT_ACCEPT()
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

    IRIdentifierExpression(std::string const& value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRStructExpression : public IRExpression
{
    std::string structName;
    std::vector<std::pair<std::string, IRExpression*>> fields;

    IRStructExpression(
        std::string const& structName,
        std::vector<std::pair<std::string, IRExpression*>> const& fields
    )
        : structName(structName), fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRUnaryExpression : public IRExpression
{
    UnaryOperator operation;
    IRExpression* expression;
    IRFunctionDeclaration* target;

    IRUnaryExpression(
        UnaryOperator operation,
        IRExpression* expression,
        IRFunctionDeclaration* target
    )
        : operation(operation), expression(expression), target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRBinaryExpression : public IRExpression
{
    BinaryOperator operation;
    IRExpression *left, *right;
    IRFunctionDeclaration* target;

    IRBinaryExpression(
        BinaryOperator operation,
        IRExpression* left,
        IRExpression* right,
        IRFunctionDeclaration* target
    )
        : operation(operation), left(left), right(right), target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRCallExpression : public IRExpression
{
    IRExpression* expression;
    std::vector<IRExpression*> args;
    IRFunctionDeclaration* target;

    IRCallExpression(
        IRExpression* expression,
        std::vector<IRExpression*> const& args,
        IRFunctionDeclaration* target
    )
        : expression(expression), args(args), target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRIndexExpression : public IRExpression
{
    IRExpression* expression;
    std::vector<IRExpression*> args;
    IRFunctionDeclaration* target;

    IRIndexExpression(
        IRExpression* expression,
        std::vector<IRExpression*> const& args,
        IRFunctionDeclaration* target
    )
        : expression(expression), args(args), target(target)
    {
    }

    IMPLEMENT_ACCEPT()
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