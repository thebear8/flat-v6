#pragma once
#include <vector>
#include <string>
#include <unordered_map>

#include "operator.hpp"
#include "../type/type.hpp"
#include "../util/visitor.hpp"

using IRTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
	struct IRNode,
	struct IRDeclaration,
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
	struct IRStructDeclaration,
	struct IRFunctionDeclaration,
	struct IRExternFunctionDeclaration,
	struct IRSourceFile
>;

template<typename TReturn>
using IRVisitor = IRTripleDispatchVisitor::Visitor<TReturn>;

struct IRNode : IRTripleDispatchVisitor::NodeBase
{
	IMPLEMENT_ACCEPT()
};

struct IRDeclaration : public IRNode
{
	IMPLEMENT_ACCEPT()
};

struct IRStatement : public IRNode
{
	IMPLEMENT_ACCEPT()
};

struct IRExpression : public IRNode
{
	Type* type;

	IRExpression() :
		type(nullptr) {}

	IMPLEMENT_ACCEPT()
};

//

struct IRIntegerExpression : public IRExpression
{
	bool isSigned;
	size_t width, radix;
	std::string value;

	IRIntegerExpression(bool isSigned, size_t width, size_t radix, std::string const& value) :
		isSigned(isSigned), width(width), radix(radix), value(value) {}

	IMPLEMENT_ACCEPT()
};

struct IRBoolExpression : public IRExpression
{
	bool value;

	IRBoolExpression(bool value) :
		value(value) {}

	IMPLEMENT_ACCEPT()
};

struct IRCharExpression : public IRExpression
{
	uint32_t value;

	IRCharExpression(uint32_t value) :
		value(value) {}

	IMPLEMENT_ACCEPT()
};

struct IRStringExpression : public IRExpression
{
	std::vector<uint8_t> value;

	IRStringExpression(std::vector<uint8_t> const& value) :
		value(value) {}

	IMPLEMENT_ACCEPT()
};

struct IRIdentifierExpression : public IRExpression
{
	std::string value;

	IRIdentifierExpression(std::string const& value) :
		value(value) {}

	IMPLEMENT_ACCEPT()
};

struct IRStructExpression : public IRExpression
{
	std::string structName;
	std::vector<std::pair<std::string, IRExpression*>> fields;

	IRStructExpression(std::string const& structName, std::vector<std::pair<std::string, IRExpression*>> const& fields) :
		structName(structName), fields(fields) {}

	IMPLEMENT_ACCEPT()
};

struct IRUnaryExpression : public IRExpression
{
	UnaryOperator operation;
	IRExpression* expression;

	IRUnaryExpression(UnaryOperator operation, IRExpression* expression) :
		operation(operation), expression(expression) {}

	IMPLEMENT_ACCEPT()
};

struct IRBinaryExpression : public IRExpression
{
	BinaryOperator operation;
	IRExpression* left, * right;

	IRBinaryExpression(BinaryOperator operation, IRExpression* left, IRExpression* right) : 
		operation(operation), left(left), right(right) {}

	IMPLEMENT_ACCEPT()
};

struct IRCallExpression : public IRExpression
{
	IRExpression* expression;
	std::vector<IRExpression*> args;

	IRCallExpression(IRExpression* expression, std::vector<IRExpression*> const& args) :
		expression(expression), args(args) {}

	IMPLEMENT_ACCEPT()
};

struct IRIndexExpression : public IRExpression
{
	IRExpression* expression;
	std::vector<IRExpression*> args;

	IRIndexExpression(IRExpression* expression, std::vector<IRExpression*> const& args) :
		expression(expression), args(args) {}

	IMPLEMENT_ACCEPT()
};

struct IRFieldExpression : public IRExpression
{
	IRExpression* expression;
	std::string fieldName;

	IRFieldExpression(IRExpression* expression, std::string const& fieldName) :
		expression(expression), fieldName(fieldName) {}

	IMPLEMENT_ACCEPT()
};

//

struct IRBlockStatement : public IRStatement
{
	std::vector<IRStatement*> statements;

	IRBlockStatement(std::vector<IRStatement*> const& statements) :
		statements(statements) {}

	IMPLEMENT_ACCEPT()
};

struct IRExpressionStatement : public IRStatement
{
	IRExpression* expression;

	IRExpressionStatement(IRExpression* expression) :
		expression(expression) {}

	IMPLEMENT_ACCEPT()
};

struct IRVariableStatement : public IRStatement
{
	std::vector<std::pair<std::string, IRExpression*>> items;

	IRVariableStatement(std::vector<std::pair<std::string, IRExpression*>> const& items) :
		items(items) {}

	IMPLEMENT_ACCEPT()
};

struct IRReturnStatement : public IRStatement
{
	IRExpression* expression;

	IRReturnStatement(IRExpression* expression) :
		expression(expression) {}

	IMPLEMENT_ACCEPT()
};

struct IRWhileStatement : public IRStatement
{
	IRExpression* condition;
	IRStatement* body;

	IRWhileStatement(IRExpression* condition, IRStatement* body) :
		condition(condition), body(body) {}

	IMPLEMENT_ACCEPT()
};

struct IRIfStatement : public IRStatement
{
	IRExpression* condition;
	IRStatement* ifBody, *elseBody;

	IRIfStatement(IRExpression* condition, IRStatement* ifBody, IRStatement* elseBody) :
		condition(condition), ifBody(ifBody), elseBody(elseBody) {}

	IMPLEMENT_ACCEPT()
};

//

struct IRStructDeclaration : public IRDeclaration
{
	std::string name;
	std::vector<std::pair<std::string, Type*>> fields;

	IRStructDeclaration(std::string const& name, std::vector<std::pair<std::string, Type*>> const& fields) :
		name(name), fields(fields) {}

	IMPLEMENT_ACCEPT()
};

struct IRFunctionDeclaration : public IRDeclaration
{
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> params;
	IRStatement* body;

	IRFunctionDeclaration(std::string const& name, Type* result, std::vector<std::pair<std::string, Type*>> const& params, IRStatement* body) :
		name(name), result(result), params(params), body(body) {}

	IMPLEMENT_ACCEPT()
};

struct IRExternFunctionDeclaration : public IRDeclaration
{
	std::string lib;
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> params;

	IRExternFunctionDeclaration(std::string const& lib, std::string const& name, Type* result, std::vector<std::pair<std::string, Type*>> const& params) :
		lib(lib), name(name), result(result), params(params) {}

	IMPLEMENT_ACCEPT()
};

//

struct IRSourceFile : public IRNode
{
	std::vector<std::string> path;
	std::vector<std::vector<std::string>> imports;
	std::vector<IRDeclaration*> declarations;

	IRSourceFile(std::vector<std::string> const& path, std::vector<std::vector<std::string>> const& imports, std::vector<IRDeclaration*> declarations) :
		path(path), imports(imports), declarations() {}

	IMPLEMENT_ACCEPT()
};