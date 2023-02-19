#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../util/md_container.hpp"
#include "../util/visitor.hpp"
#include "operator.hpp"
#include "source_ref.hpp"

using IRTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
    struct IRNode,

    struct IRExpression,
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

    struct IRStatement,
    struct IRExpressionStatement,
    struct IRVariableStatement,
    struct IRReturnStatement,
    struct IRWhileStatement,
    struct IRIfStatement,

    struct IRDeclaration,
    struct IRConstraintDeclaration,
    struct IRStructDeclaration,
    struct IRFunctionDeclaration,
    struct IRSourceFile,

    struct IRType,
    struct IRGenericType,
    struct IRVoidType,
    struct IRBoolType,
    struct IRIntegerType,
    struct IRCharType,
    struct IRStringType,
    struct IRStructType,
    struct IRPointerType,
    struct IRArrayType>;

using IRMetadataContainer = MetadataContainer<SourceRef, IRType*>;

template<typename TReturn>
using IRVisitor = IRTripleDispatchVisitor::Visitor<TReturn>;

struct IRNode : IRTripleDispatchVisitor::NodeBase, private IRMetadataContainer {
    IMPLEMENT_ACCEPT()

    template<typename TValue>
    std::optional<TValue> const& getMD() {
        return IRMetadataContainer::setMD<TValue>();
    }

    template<typename TValue>
    IRNode* setMD(TValue&& v) {
        IRMetadataContainer::setMD<TValue>(std::forward<TValue>(v));
        return this;
    }

    template<typename TValue>
    IRNode* setMD(TValue const& v) {
        IRMetadataContainer::setMD<TValue>(std::forward<TValue>(v));
        return this;
    }
};

//

struct IRExpression : public IRNode {
    IMPLEMENT_ACCEPT()
};

struct IRIntegerExpression : public IRExpression {
    bool isSigned;
    size_t width, radix;
    std::string value;

    IRIntegerExpression(
        bool isSigned, size_t width, size_t radix, std::string const& value
    )
        : isSigned(isSigned), width(width), radix(radix), value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRBoolExpression : public IRExpression {
    bool value;

    IRBoolExpression(bool value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRCharExpression : public IRExpression {
    uint32_t value;

    IRCharExpression(uint32_t value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRStringExpression : public IRExpression {
    std::vector<uint8_t> value;

    IRStringExpression(std::vector<uint8_t> const& value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRIdentifierExpression : public IRExpression {
    std::string value;

    IRIdentifierExpression(std::string const& value) : value(value) {}

    IMPLEMENT_ACCEPT()
};

struct IRStructExpression : public IRExpression {
    std::string structName;
    std::vector<std::pair<std::string, IRExpression*>> fields;

    IRStructExpression(
        std::string const& structName,
        std::vector<std::pair<std::string, IRExpression*>> const& fields
    )
        : structName(structName), fields(fields) {}

    IMPLEMENT_ACCEPT()
};

struct IRUnaryExpression : public IRExpression {
    UnaryOperator operation;
    IRExpression* expression;
    IRFunctionDeclaration* target;

    IRUnaryExpression(
        UnaryOperator operation,
        IRExpression* expression,
        IRFunctionDeclaration* target
    )
        : operation(operation), expression(expression), target(target) {}

    IMPLEMENT_ACCEPT()
};

struct IRBinaryExpression : public IRExpression {
    BinaryOperator operation;
    IRExpression *left, *right;
    IRFunctionDeclaration* target;

    IRBinaryExpression(
        BinaryOperator operation,
        IRExpression* left,
        IRExpression* right,
        IRFunctionDeclaration* target
    )
        : operation(operation), left(left), right(right), target(target) {}

    IMPLEMENT_ACCEPT()
};

struct IRCallExpression : public IRExpression {
    IRExpression* expression;
    std::vector<IRExpression*> args;
    IRFunctionDeclaration* target;

    IRCallExpression(
        IRExpression* expression,
        std::vector<IRExpression*> const& args,
        IRFunctionDeclaration* target
    )
        : expression(expression), args(args), target(target) {}

    IMPLEMENT_ACCEPT()
};

struct IRIndexExpression : public IRExpression {
    IRExpression* expression;
    std::vector<IRExpression*> args;
    IRFunctionDeclaration* target;

    IRIndexExpression(
        IRExpression* expression,
        std::vector<IRExpression*> const& args,
        IRFunctionDeclaration* target
    )
        : expression(expression), args(args), target(target) {}

    IMPLEMENT_ACCEPT()
};

struct IRFieldExpression : public IRExpression {
    IRExpression* expression;
    std::string fieldName;

    IRFieldExpression(IRExpression* expression, std::string const& fieldName)
        : expression(expression), fieldName(fieldName) {}

    IMPLEMENT_ACCEPT()
};

//

struct IRStatement : public IRNode {
    IMPLEMENT_ACCEPT()
};

struct IRBlockStatement : public IRStatement {
    std::vector<IRStatement*> statements;

    IRBlockStatement(std::vector<IRStatement*> const& statements)
        : statements(statements) {}

    IMPLEMENT_ACCEPT()
};

struct IRExpressionStatement : public IRStatement {
    IRExpression* expression;

    IRExpressionStatement(IRExpression* expression) : expression(expression) {}

    IMPLEMENT_ACCEPT()
};

struct IRVariableStatement : public IRStatement {
    std::vector<std::pair<std::string, IRExpression*>> items;

    IRVariableStatement(
        std::vector<std::pair<std::string, IRExpression*>> const& items
    )
        : items(items) {}

    IMPLEMENT_ACCEPT()
};

struct IRReturnStatement : public IRStatement {
    IRExpression* expression;

    IRReturnStatement(IRExpression* expression) : expression(expression) {}

    IMPLEMENT_ACCEPT()
};

struct IRWhileStatement : public IRStatement {
    IRExpression* condition;
    IRStatement* body;

    IRWhileStatement(IRExpression* condition, IRStatement* body)
        : condition(condition), body(body) {}

    IMPLEMENT_ACCEPT()
};

struct IRIfStatement : public IRStatement {
    IRExpression* condition;
    IRStatement *ifBody, *elseBody;

    IRIfStatement(
        IRExpression* condition, IRStatement* ifBody, IRStatement* elseBody
    )
        : condition(condition), ifBody(ifBody), elseBody(elseBody) {}

    IMPLEMENT_ACCEPT()
};

//

struct IRDeclaration : public IRNode {
    std::vector<IRGenericType*> typeParams;
    std::vector<std::pair<std::string, std::vector<IRType*>>> requirements;

    IRDeclaration(
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements
    )
        : typeParams(typeParams), requirements(requirements) {}

    IMPLEMENT_ACCEPT()
};

struct IRConstraintDeclaration : public IRDeclaration {
    std::string name;
    std::vector<IRDeclaration*> conditions;

    IRConstraintDeclaration(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        std::vector<IRDeclaration*> const& conditions
    )
        : IRDeclaration(typeParams, requirements),
          name(name),
          conditions(conditions) {}

    IMPLEMENT_ACCEPT()
};

struct IRStructDeclaration : public IRDeclaration {
    std::string name;
    std::vector<std::pair<std::string, IRType*>> fields;

    IRStructDeclaration(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        std::vector<std::pair<std::string, IRType*>> const& fields
    )
        : IRDeclaration(typeParams, requirements), name(name), fields(fields) {}

    IMPLEMENT_ACCEPT()
};

struct IRFunctionDeclaration : public IRDeclaration {
    std::string lib;
    std::string name;
    IRType* result;
    std::vector<std::pair<std::string, IRType*>> params;
    IRStatement* body;

    IRFunctionDeclaration(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        IRType* result,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRStatement* body
    )
        : IRDeclaration(typeParams, requirements),
          lib(""),
          name(name),
          result(result),
          params(params),
          body(body) {}

    IRFunctionDeclaration(
        std::string const& lib,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        IRType* result,
        std::vector<std::pair<std::string, IRType*>> const& params
    )
        : IRDeclaration(typeParams, requirements),
          lib(lib),
          name(name),
          result(result),
          params(params),
          body(nullptr) {}

    IMPLEMENT_ACCEPT()
};

//

struct IRType : public IRNode {
    virtual size_t getBitSize() {
        throw std::exception("getBitSize() called on type that has no size");
    };

    virtual std::string toString() = 0;

    virtual bool isGenericType() { return false; }
    virtual bool isVoidType() { return false; }
    virtual bool isBoolType() { return false; }
    virtual bool isIntegerType() { return false; }
    virtual bool isCharType() { return false; }
    virtual bool isStringType() { return false; }
    virtual bool isStructType() { return false; }
    virtual bool isPointerType() { return false; }
    virtual bool isArrayType() { return false; }
    virtual bool isSigned() { return false; }

    IMPLEMENT_ACCEPT()
};

struct IRGenericType : public IRType {
    std::string name;

    IRGenericType(std::string const& name) : name(name) {}

    virtual std::string toString() override { return name; }
    virtual bool isGenericType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRVoidType : public IRType {
    virtual std::string toString() override { return "void"; }
    virtual bool isVoidType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRBoolType : public IRType {
    virtual size_t getBitSize() override { return 1; };
    virtual std::string toString() override { return "bool"; };
    virtual bool isBoolType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRIntegerType : public IRType {
    bool signedness;
    size_t bitSize;

    IRIntegerType(bool signedness, size_t bitSize)
        : signedness(signedness), bitSize(bitSize) {}

    virtual size_t getBitSize() override { return bitSize; };

    virtual std::string toString() override {
        return (signedness ? "i" : "u") + std::to_string(bitSize);
    };

    virtual bool isIntegerType() override { return true; }
    virtual bool isSigned() override { return signedness; }

    IMPLEMENT_ACCEPT()
};

struct IRCharType : public IRType {
    virtual size_t getBitSize() override { return 32; };
    virtual std::string toString() override { return "char"; };
    virtual bool isCharType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRStringType : public IRType {
    virtual std::string toString() override { return "str"; }
    virtual bool isStringType() override { return true; }
};

struct IRStructType : public IRType {
    std::string name;
    std::vector<std::pair<std::string, IRType*>> fields;

    IRStructType(
        std::string const& name,
        std::vector<std::pair<std::string, IRType*>> const& fields
    )
        : name(name), fields(fields) {}

    virtual std::string toString() override { return name; };
    virtual bool isStructType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRPointerType : public IRType {
    IRType* base;

    IRPointerType(IRType* base) : base(base) {}

    virtual std::string toString() override { return base->toString() + "*"; };
    virtual bool isPointerType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRArrayType : public IRType {
    IRType* base;

    IRArrayType(IRType* base) : base(base) {}

    virtual std::string toString() override { return base->toString() + "[]"; };
    virtual bool isArrayType() override { return true; }

    IMPLEMENT_ACCEPT()
};

//

struct IRSourceFile : public IRNode {
    std::string path;
    std::vector<std::string> imports;
    std::vector<IRDeclaration*> declarations;

    IRSourceFile(
        std::string const& path,
        std::vector<std::string> const& imports,
        std::vector<IRDeclaration*> const& declarations
    )
        : path(path), imports(imports), declarations(declarations) {}

    IMPLEMENT_ACCEPT()
};