#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

struct ASTStatement : public ASTNode
{
    ASTStatement(SourceRef const& location) : ASTNode(location) {}

    IMPLEMENT_ACCEPT()
};

struct ASTBlockStatement : public ASTStatement
{
    std::vector<ASTStatement*> statements;

    ASTBlockStatement(
        SourceRef const& location, std::vector<ASTStatement*> statements
    )
        : ASTStatement(location), statements(statements)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTExpressionStatement : public ASTStatement
{
    ASTExpression* expression;

    ASTExpressionStatement(SourceRef const& location, ASTExpression* expression)
        : ASTStatement(location), expression(expression)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTVariableStatement : public ASTStatement
{
    std::vector<std::pair<std::string, ASTExpression*>> items;

    ASTVariableStatement(
        SourceRef const& location,
        std::vector<std::pair<std::string, ASTExpression*>> const& items
    )
        : ASTStatement(location), items(items)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTReturnStatement : public ASTStatement
{
    ASTExpression* expression;

    ASTReturnStatement(SourceRef const& location, ASTExpression* expression)
        : ASTStatement(location), expression(expression)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTWhileStatement : public ASTStatement
{
    ASTExpression* condition;
    ASTStatement* body;

    ASTWhileStatement(
        SourceRef const& location, ASTExpression* condition, ASTStatement* body
    )
        : ASTStatement(location), condition(condition), body(body)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct ASTIfStatement : public ASTStatement
{
    ASTExpression* condition;
    ASTStatement *ifBody, *elseBody;

    ASTIfStatement(
        SourceRef const& location,
        ASTExpression* condition,
        ASTStatement* ifBody,
        ASTStatement* elseBody
    )
        : ASTStatement(location),
          condition(condition),
          ifBody(ifBody),
          elseBody(elseBody)
    {
    }

    IMPLEMENT_ACCEPT()
};