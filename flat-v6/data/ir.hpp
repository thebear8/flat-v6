#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../util/visitor.hpp"
#include "operator.hpp"
#include "type.hpp"

using IRTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
    struct IRNode,
    struct IRDefinition,
    struct IRStatement,
    struct IRExpression,
    struct IRStatement,
    struct IRIntegerExpression,
    struct IRBoolExpression,
    struct IRCharExpression,
    struct IRStringExpression,
    struct IRIdentifierExpression,
    struct IRStructExpression,
    struct IRUnaryExpression,
    struct IRBinaryExpression,
    struct IRCallExpression,
    struct IRIndexExpression,
    struct IRFieldExpression,
    struct IRBlockStatement,
    struct IRExpressionStatement,
    struct IRVariableStatement,
    struct IRReturnStatement,
    struct IRWhileStatement,
    struct IRIfStatement,
    struct IRStructDefinition,
    struct IRFunctionDefinition,
    struct IRSourceFile>;

template<typename TReturn>
using IRVisitor = IRTripleDispatchVisitor::Visitor<TReturn>;

struct IRNode : IRTripleDispatchVisitor::NodeBase
{
    SourceRef location;

    IRNode(SourceRef const& location) : location(location) {}

    IMPLEMENT_ACCEPT()
};

struct IRDefinition : public IRNode
{
    IRDefinition(SourceRef const& location) : IRNode(location) {}

    IMPLEMENT_ACCEPT()
};

struct IRStatement : public IRNode
{
    IRStatement(SourceRef const& location) : IRNode(location) {}

    IMPLEMENT_ACCEPT()
};

struct IRExpression : public IRNode
{
    Type* type;

    IRExpression(SourceRef const& location) : IRNode(location), type(nullptr) {}

    IMPLEMENT_ACCEPT()
};

//

struct IRIntegerExpression : public IRExpression
{
    bool isSigned;
    size_t width, radix;
    std::string value;

    IRIntegerExpression(
        SourceRef const& location,
        bool isSigned,
        size_t width,
        size_t radix,
        std::string const& value)
        : IRExpression(location),
          isSigned(isSigned),
          width(width),
          radix(radix),
          value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRBoolExpression : public IRExpression
{
    bool value;

    IRBoolExpression(SourceRef const& location, bool value)
        : IRExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRCharExpression : public IRExpression
{
    uint32_t value;

    IRCharExpression(SourceRef const& location, uint32_t value)
        : IRExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRStringExpression : public IRExpression
{
    std::vector<uint8_t> value;

    IRStringExpression(
        SourceRef const& location, std::vector<uint8_t> const& value)
        : IRExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRIdentifierExpression : public IRExpression
{
    std::string value;

    IRIdentifierExpression(SourceRef const& location, std::string const& value)
        : IRExpression(location), value(value)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRStructExpression : public IRExpression
{
    std::string structName;
    std::vector<std::pair<std::string, IRExpression*>> fields;

    IRStructExpression(
        SourceRef const& location,
        std::string const& structName,
        std::vector<std::pair<std::string, IRExpression*>> const& fields)
        : IRExpression(location), structName(structName), fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRUnaryExpression : public IRExpression
{
    UnaryOperator operation;
    IRExpression* expression;
    IRFunctionDefinition* target;

    IRUnaryExpression(
        SourceRef const& location,
        UnaryOperator operation,
        IRExpression* expression,
        IRFunctionDefinition* target)
        : IRExpression(location),
          operation(operation),
          expression(expression),
          target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRBinaryExpression : public IRExpression
{
    BinaryOperator operation;
    IRExpression *left, *right;
    IRFunctionDefinition* target;

    IRBinaryExpression(
        SourceRef const& location,
        BinaryOperator operation,
        IRExpression* left,
        IRExpression* right,
        IRFunctionDefinition* target)
        : IRExpression(location),
          operation(operation),
          left(left),
          right(right),
          target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRCallExpression : public IRExpression
{
    IRExpression* expression;
    std::vector<IRExpression*> args;
    IRFunctionDefinition* target;

    IRCallExpression(
        SourceRef const& location,
        IRExpression* expression,
        std::vector<IRExpression*> const& args,
        IRFunctionDefinition* target)
        : IRExpression(location),
          expression(expression),
          args(args),
          target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRIndexExpression : public IRExpression
{
    IRExpression* expression;
    std::vector<IRExpression*> args;
    IRFunctionDefinition* target;

    IRIndexExpression(
        SourceRef const& location,
        IRExpression* expression,
        std::vector<IRExpression*> const& args,
        IRFunctionDefinition* target)
        : IRExpression(location),
          expression(expression),
          args(args),
          target(target)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRFieldExpression : public IRExpression
{
    IRExpression* expression;
    std::string fieldName;

    IRFieldExpression(
        SourceRef const& location,
        IRExpression* expression,
        std::string const& fieldName)
        : IRExpression(location), expression(expression), fieldName(fieldName)
    {
    }

    IMPLEMENT_ACCEPT()
};

//

struct IRBlockStatement : public IRStatement
{
    std::vector<IRStatement*> statements;

    IRBlockStatement(
        SourceRef const& location, std::vector<IRStatement*> const& statements)
        : IRStatement(location), statements(statements)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRExpressionStatement : public IRStatement
{
    IRExpression* expression;

    IRExpressionStatement(SourceRef const& location, IRExpression* expression)
        : IRStatement(location), expression(expression)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRVariableStatement : public IRStatement
{
    std::vector<std::pair<std::string, IRExpression*>> items;

    IRVariableStatement(
        SourceRef const& location,
        std::vector<std::pair<std::string, IRExpression*>> const& items)
        : IRStatement(location), items(items)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRReturnStatement : public IRStatement
{
    IRExpression* expression;

    IRReturnStatement(SourceRef const& location, IRExpression* expression)
        : IRStatement(location), expression(expression)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRWhileStatement : public IRStatement
{
    IRExpression* condition;
    IRStatement* body;

    IRWhileStatement(
        SourceRef const& location, IRExpression* condition, IRStatement* body)
        : IRStatement(location), condition(condition), body(body)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRIfStatement : public IRStatement
{
    IRExpression* condition;
    IRStatement *ifBody, *elseBody;

    IRIfStatement(
        SourceRef const& location,
        IRExpression* condition,
        IRStatement* ifBody,
        IRStatement* elseBody)
        : IRStatement(location),
          condition(condition),
          ifBody(ifBody),
          elseBody(elseBody)
    {
    }

    IMPLEMENT_ACCEPT()
};

//

struct IRStructDefinition : public IRDefinition
{
    std::string name;
    std::vector<std::pair<std::string, Type*>> fields;

    IRStructDefinition(
        SourceRef const& location,
        std::string const& name,
        std::vector<std::pair<std::string, Type*>> const& fields)
        : IRDefinition(location), name(name), fields(fields)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRFunctionDefinition : public IRDefinition
{
    std::string lib;
    std::string name;
    Type* result;
    std::vector<std::pair<std::string, Type*>> params;
    IRStatement* body;

    IRFunctionDefinition(
        SourceRef const& location,
        std::string const& name,
        Type* result,
        std::vector<std::pair<std::string, Type*>> const& params,
        IRStatement* body)
        : IRDefinition(location),
          lib(""),
          name(name),
          result(result),
          params(params),
          body(body)
    {
    }

    IRFunctionDefinition(
        SourceRef const& location,
        std::string const& lib,
        std::string const& name,
        Type* result,
        std::vector<std::pair<std::string, Type*>> const& params)
        : IRDefinition(location),
          lib(lib),
          name(name),
          result(result),
          params(params),
          body(nullptr)
    {
    }

    IMPLEMENT_ACCEPT()
};

//

struct IRSourceFile : public IRNode
{
    std::string path;
    std::vector<std::string> imports;
    std::vector<IRDefinition*> definitions;

    IRSourceFile(
        SourceRef const& location,
        std::string const& path,
        std::vector<std::string> const& imports,
        std::vector<IRDefinition*> const& definitions)
        : IRNode(location),
          path(path),
          imports(imports),
          definitions(definitions)
    {
    }

    IMPLEMENT_ACCEPT()
};