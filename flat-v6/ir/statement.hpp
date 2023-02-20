#pragma once
#include <string>
#include <vector>

#include "ir_node.hpp"

struct IRStatement : public IRNode
{
    IMPLEMENT_ACCEPT()
};

struct IRBlockStatement : public IRStatement
{
    std::vector<IRStatement*> statements;

    IRBlockStatement(std::vector<IRStatement*> const& statements)
        : statements(statements)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRExpressionStatement : public IRStatement
{
    IRExpression* expression;

    IRExpressionStatement(IRExpression* expression) : expression(expression) {}

    IMPLEMENT_ACCEPT()
};

struct IRVariableStatement : public IRStatement
{
    std::vector<std::pair<std::string, IRExpression*>> items;

    IRVariableStatement(
        std::vector<std::pair<std::string, IRExpression*>> const& items
    )
        : items(items)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRReturnStatement : public IRStatement
{
    IRExpression* expression;

    IRReturnStatement(IRExpression* expression) : expression(expression) {}

    IMPLEMENT_ACCEPT()
};

struct IRWhileStatement : public IRStatement
{
    IRExpression* condition;
    IRStatement* body;

    IRWhileStatement(IRExpression* condition, IRStatement* body)
        : condition(condition), body(body)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRIfStatement : public IRStatement
{
    IRExpression* condition;
    IRStatement *ifBody, *elseBody;

    IRIfStatement(
        IRExpression* condition, IRStatement* ifBody, IRStatement* elseBody
    )
        : condition(condition), ifBody(ifBody), elseBody(elseBody)
    {
    }

    IMPLEMENT_ACCEPT()
};