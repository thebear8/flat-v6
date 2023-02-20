#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"
#include "operator.hpp"

struct ASTExpression : public ASTNode
{
    ASTType* type;

    ASTExpression(SourceRef const& location) : ASTNode(location), type(nullptr)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTIntegerExpression : public ASTExpression
{
    std::string value;
    std::string suffix;

    ASTIntegerExpression(
        SourceRef const& location,
        std::string const& value,
        std::string const& suffix
    )
        : ASTExpression(location), value(value), suffix(suffix)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTBoolExpression : public ASTExpression
{
    std::string value;

    ASTBoolExpression(SourceRef const& location, std::string const& value)
        : ASTExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTCharExpression : public ASTExpression
{
    std::string value;

    ASTCharExpression(SourceRef const& location, std::string const& value)
        : ASTExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTStringExpression : public ASTExpression
{
    std::string value;

    ASTStringExpression(SourceRef const& location, std::string const& value)
        : ASTExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTIdentifierExpression : public ASTExpression
{
    std::string value;

    ASTIdentifierExpression(SourceRef const& location, std::string const& value)
        : ASTExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTStructExpression : public ASTExpression
{
    std::string structName;
    std::vector<std::pair<std::string, ASTExpression*>> fields;

    ASTStructExpression(
        SourceRef const& location,
        std::string const& structName,
        std::vector<std::pair<std::string, ASTExpression*>> const& fields
    )
        : ASTExpression(location), structName(structName), fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTUnaryExpression : public ASTExpression
{
    UnaryOperator operation;
    ASTExpression* expression;

    ASTUnaryExpression(
        SourceRef const& location,
        UnaryOperator operation,
        ASTExpression* expression
    )
        : ASTExpression(location), operation(operation), expression(expression)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTBinaryExpression : public ASTExpression
{
    BinaryOperator operation;
    ASTExpression *left, *right;

    ASTBinaryExpression(
        SourceRef const& location,
        BinaryOperator operation,
        ASTExpression* left,
        ASTExpression* right
    )
        : ASTExpression(location),
          operation(operation),
          left(left),
          right(right)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTCallExpression : public ASTExpression
{
    ASTExpression* expression;
    std::vector<ASTExpression*> args;

    ASTCallExpression(
        SourceRef const& location,
        ASTExpression* expression,
        std::vector<ASTExpression*> const& args
    )
        : ASTExpression(location), expression(expression), args(args)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTIndexExpression : public ASTExpression
{
    ASTExpression* expression;
    std::vector<ASTExpression*> args;

    ASTIndexExpression(
        SourceRef const& location,
        ASTExpression* expression,
        std::vector<ASTExpression*> const& args
    )
        : ASTExpression(location), expression(expression), args(args)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTFieldExpression : public ASTExpression
{
    ASTExpression* expression;
    std::string fieldName;

    ASTFieldExpression(
        SourceRef const& location,
        ASTExpression* expression,
        std::string const& fieldName
    )
        : ASTExpression(location), expression(expression), fieldName(fieldName)
    {
    }

    IMPLEMENT_ACCEPT()
};