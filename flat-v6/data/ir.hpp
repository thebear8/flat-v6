#pragma once
#include <vector>
#include <string>
#include <unordered_map>

#include "operator.hpp"
#include "../type/type.hpp"
#include "../util/visitor.hpp"

using IRTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
	struct IRNode
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
	Type* type = nullptr;

	IMPLEMENT_ACCEPT()
};

//

struct IRIntegerExpression : public IRExpression
{
	std::string value;

	IMPLEMENT_ACCEPT()
};

struct IRBoolExpression : public IRExpression
{
	bool value;

	IMPLEMENT_ACCEPT()
};

struct IRCharExpression : public IRExpression
{
	uint32_t value;

	IMPLEMENT_ACCEPT()
};

struct IRStringExpression : public IRExpression
{
	std::string value;

	IMPLEMENT_ACCEPT()
};

struct IRIdentifierExpression : public IRExpression
{
	std::string value;

	IMPLEMENT_ACCEPT()
};

struct IRStructExpression : public IRExpression
{
	std::string structName;
	std::vector<std::pair<std::string, IRExpression*>> fields;

	IMPLEMENT_ACCEPT()
};

struct IRUnaryExpression : public IRExpression
{
	UnaryOperator operation;
	IRExpression* expression;

	IMPLEMENT_ACCEPT()
};

struct IRBinaryExpression : public IRExpression
{
	BinaryOperator operation;
	IRExpression* left, * right;

	IMPLEMENT_ACCEPT()
};

struct IRCallExpression : public IRExpression
{
	IRExpression* expression;
	std::vector<IRExpression*> args;

	IMPLEMENT_ACCEPT()
};

struct IRIndexExpression : public IRExpression
{
	IRExpression* expression;
	std::vector<IRExpression*> args;

	IMPLEMENT_ACCEPT()
};

struct IRFieldExpression : public IRExpression
{
	IRExpression* expression;
	std::string fieldName;

	IMPLEMENT_ACCEPT()
};

//

struct IRBlockStatement : public IRStatement
{
	std::vector<IRStatement*> statements;

	IMPLEMENT_ACCEPT()
};

struct IRExpressionStatement : public IRStatement
{
	IRExpression* expression;

	IMPLEMENT_ACCEPT()
};

struct IRVariableStatement : public IRStatement
{
	std::vector<std::pair<std::string, IRExpression*>> items;

	IMPLEMENT_ACCEPT()
};

struct IRReturnStatement : public IRStatement
{
	IRExpression* expression;

	IMPLEMENT_ACCEPT()
};

struct IRWhileStatement : public IRStatement
{
	IRExpression* condition;
	IRStatement* body;

	IMPLEMENT_ACCEPT()
};

struct IRIfStatement : public IRStatement
{
	IRExpression* condition;
	IRStatement* body;

	IMPLEMENT_ACCEPT()
};

//

struct IRStructDeclaration : public IRDeclaration
{
	std::string name;
	std::vector<std::pair<std::string, Type*>> fields;

	IMPLEMENT_ACCEPT()
};

struct IRFunctionDeclaration : public IRDeclaration
{
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> params;
	IRStatement* body;

	IMPLEMENT_ACCEPT()
};

struct IRExternFunctionDeclaration : public IRDeclaration
{
	std::string lib;
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> params;

	IMPLEMENT_ACCEPT()
};

//

struct IRSourceFile : public IRNode
{
	std::vector<std::string> path;
	std::vector<std::vector<std::string>> imports;
	std::vector<IRDeclaration*> declarations;
};