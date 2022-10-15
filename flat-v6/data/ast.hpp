#pragma once
#include <vector>
#include <string>
#include <unordered_map>

#include "token.hpp"
#include "operator.hpp"
#include "../type/type.hpp"
#include "../util/visitor.hpp"

using TripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
	struct AstNode,
	struct Expression,
	struct Statement,
	struct Declaration,
	struct IntegerExpression,
	struct BoolExpression,
	struct CharExpression,
	struct StringExpression,
	struct IdentifierExpression,
	struct StructExpression,
	struct UnaryExpression,
	struct BinaryExpression,
	struct CallExpression,
	struct BoundCallExpression,
	struct IndexExpression,
	struct BoundIndexExpression,
	struct FieldExpression,
	struct BlockStatement,
	struct ExpressionStatement,
	struct VariableStatement,
	struct ReturnStatement,
	struct WhileStatement,
	struct IfStatement,
	struct StructDeclaration,
	struct FunctionDeclaration,
	struct ExternFunctionDeclaration,
	struct Module
>;

using AstNodeBase = TripleDispatchVisitor::AstNode;
template<typename TReturn>
using Visitor = TripleDispatchVisitor::Visitor<TReturn>;

struct AstNode : public AstNodeBase
{
	size_t begin, end;

	AstNode() :
		begin(0), end(0) { }

	IMPLEMENT_ACCEPT()
};

struct Declaration : public AstNode
{
	IMPLEMENT_ACCEPT()
};

struct Statement : public AstNode
{
	IMPLEMENT_ACCEPT()
};

struct Expression : public AstNode
{
	Type* type;

	Expression() :
		type(nullptr) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct IntegerExpression : public Expression
{
	std::string value;
	std::string suffix;

	IntegerExpression(std::string const& value, std::string const& suffix) :
		value(value), suffix(suffix) { }

	IMPLEMENT_ACCEPT()
};

struct BoolExpression : public Expression
{
	std::string value;

	BoolExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct CharExpression : public Expression
{
	std::string value;

	CharExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct StringExpression : public Expression
{
	std::string value;

	StringExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct IdentifierExpression : public Expression
{
	std::string value;

	IdentifierExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct StructExpression : public Expression
{
	std::string structName;
	std::vector<std::pair<std::string, Expression*>> fields;

	StructExpression(std::string const& structName, std::vector<std::pair<std::string, Expression*>> const& fields) :
		structName(structName), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct UnaryExpression : public Expression
{
	UnaryOperator operation;
	Expression* expression;

	UnaryExpression(UnaryOperator operation, Expression* expression) :
		operation(operation), expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct BinaryExpression : public Expression
{
	BinaryOperator operation;
	Expression* left, *right;

	BinaryExpression(BinaryOperator operation, Expression* left, Expression* right) :
		operation(operation), left(left), right(right) { }

	IMPLEMENT_ACCEPT()
};

struct CallExpression : public Expression
{
	Expression* expression;
	std::vector<Expression*> args;

	CallExpression(Expression* expression, std::vector<Expression*> const& args) :
		expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct BoundCallExpression : public Expression
{
	std::string identifier;
	std::vector<Expression*> args;

	BoundCallExpression(std::string identifier, std::vector<Expression*> const& args) :
		identifier(identifier), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct IndexExpression : public Expression
{
	Expression* expression;
	std::vector<Expression*> args;

	IndexExpression(Expression* expression, std::vector<Expression*> const& args) :
		expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct BoundIndexExpression : public Expression
{
	Expression* expression;
	Expression* index;

	BoundIndexExpression(Expression* expression, Expression* index) :
		expression(expression), index(index) { }

	IMPLEMENT_ACCEPT()
};

struct FieldExpression : public Expression
{
	Expression* expression;
	std::string fieldName;

	FieldExpression(Expression* expression, std::string const& fieldName) :
		expression(expression), fieldName(fieldName) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct BlockStatement : public Statement
{
	std::vector<Statement*> statements;

	BlockStatement(std::vector<Statement*> statements) :
		statements(statements) { }

	IMPLEMENT_ACCEPT()
};

struct ExpressionStatement : public Statement
{
	Expression* expression;

	ExpressionStatement(Expression* expression) :
		expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct VariableStatement : public Statement
{
	std::vector<std::pair<std::string, Expression*>> items;

	VariableStatement(std::vector<std::pair<std::string, Expression*>> const& items) :
		items(items) { }

	IMPLEMENT_ACCEPT()
};

struct ReturnStatement : public Statement
{
	Expression* expression;

	ReturnStatement(Expression* expression) :
		expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct WhileStatement : public Statement
{
	Expression* condition;
	Statement* body;

	WhileStatement(Expression* condition, Statement* body) :
		condition(condition), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct IfStatement : public Statement
{
	Expression* condition;
	Statement* ifBody, * elseBody;

	IfStatement(Expression* condition, Statement* ifBody, Statement* elseBody) :
		condition(condition), ifBody(ifBody), elseBody(elseBody) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct StructDeclaration : public Declaration
{
	std::string name;
	std::vector<std::pair<std::string, Type*>> fields;

	StructDeclaration(std::string const& name, std::vector<std::pair<std::string, Type*>> const& fields) :
		name(name), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct FunctionDeclaration : public Declaration
{
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> parameters;
	Statement* body;

	FunctionDeclaration(std::string const& name, Type* result, std::vector<std::pair<std::string, Type*>> const& parameters, Statement* body) :
		name(name), result(result), parameters(parameters), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct ExternFunctionDeclaration : public Declaration
{
	std::string lib;
	std::string name;
	Type* result;
	std::vector<std::pair<std::string, Type*>> parameters;

	ExternFunctionDeclaration(std::string const& lib, std::string const& name, Type* result, std::vector<std::pair<std::string, Type*>> const& parameters) :
		lib(lib), name(name), result(result), parameters(parameters) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct Module : public AstNode
{
	std::vector<Declaration*> declarations;

	Module(std::vector<Declaration*> declarations) :
		declarations(declarations) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

class AstContext
{
private:
	std::vector<AstNode*> nodes;

public:
	AstContext() { }
	AstContext(AstContext&&) = delete;
	AstContext(AstContext const&) = delete;

	~AstContext()
	{
		for (auto node : nodes)
			delete node;
		nodes.clear();
	}

	inline void deallocate()
	{
		for (auto node : nodes)
			delete node;
		nodes.clear();
	}

public:
	template<typename TNode, typename... TArgs>
	TNode* make(size_t begin, size_t end, TArgs&&... args)
	{
		nodes.push_back(new TNode(std::forward<TArgs>(args)...));
		nodes.back()->begin = begin;
		nodes.back()->end = end;
		return static_cast<TNode*>(nodes.back());
	}
};