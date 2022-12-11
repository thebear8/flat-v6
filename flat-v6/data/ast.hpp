#pragma once
#include <vector>
#include <string>
#include <unordered_map>

#include "token.hpp"
#include "operator.hpp"
#include "../type/type.hpp"
#include "../util/visitor.hpp"
#include "../util/ast_context.hpp"

using ASTTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
	struct ASTNode,
	struct ASTExpression,
	struct ASTStatement,
	struct ASTDeclaration,
	struct ASTIntegerExpression,
	struct ASTBoolExpression,
	struct ASTCharExpression,
	struct ASTStringExpression,
	struct ASTIdentifierExpression,
	struct ASTStructExpression,
	struct ASTUnaryExpression,
	struct ASTBinaryExpression,
	struct ASTCallExpression,
	struct ASTBoundCallExpression,
	struct ASTIndexExpression,
	struct ASTBoundIndexExpression,
	struct ASTFieldExpression,
	struct ASTBlockStatement,
	struct ASTExpressionStatement,
	struct ASTVariableStatement,
	struct ASTReturnStatement,
	struct ASTWhileStatement,
	struct ASTIfStatement,
	struct ASTStructDeclaration,
	struct ASTFunctionDeclaration,
	struct ASTExternFunctionDeclaration,
	struct ASTSourceFile
> ;

template<typename TReturn>
using ASTVisitor = ASTTripleDispatchVisitor::Visitor<TReturn>;

using AstContext = ast_util::AstContext;

struct ASTNode : public ast_util::AstNodeBase, ASTTripleDispatchVisitor::NodeBase
{
	IMPLEMENT_ACCEPT()
};

struct ASTDeclaration : public ASTNode
{
	IMPLEMENT_ACCEPT()
};

struct ASTStatement : public ASTNode
{
	IMPLEMENT_ACCEPT()
};

struct ASTExpression : public ASTNode
{
	Type* type;

	ASTExpression() :
		type(nullptr) { }

	IMPLEMENT_ACCEPT()
};

struct ASTType : public ASTNode
{
	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct ASTNamedType : public ASTType
{
	std::string name;

	ASTNamedType(std::string const& name) :
		name(name) { }

	IMPLEMENT_ACCEPT()
};

struct ASTPointerType : public ASTType
{
	ASTType* base;

	ASTPointerType(ASTType* base) :
		base(base) { }

	IMPLEMENT_ACCEPT()
};

struct ASTArrayType : public ASTType
{
	ASTType* base;

	ASTArrayType(ASTType* base) :
		base(base) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct ASTIntegerExpression : public ASTExpression
{
	std::string value;
	std::string suffix;

	ASTIntegerExpression(std::string const& value, std::string const& suffix) :
		value(value), suffix(suffix) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBoolExpression : public ASTExpression
{
	std::string value;

	ASTBoolExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTCharExpression : public ASTExpression
{
	std::string value;

	ASTCharExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTStringExpression : public ASTExpression
{
	std::string value;

	ASTStringExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTIdentifierExpression : public ASTExpression
{
	std::string value;

	ASTIdentifierExpression(std::string const& value) :
		value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTStructExpression : public ASTExpression
{
	std::string structName;
	std::vector<std::pair<std::string, ASTExpression*>> fields;

	ASTStructExpression(std::string const& structName, std::vector<std::pair<std::string, ASTExpression*>> const& fields) :
		structName(structName), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct ASTUnaryExpression : public ASTExpression
{
	UnaryOperator operation;
	ASTExpression* expression;

	ASTUnaryExpression(UnaryOperator operation, ASTExpression* expression) :
		operation(operation), expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBinaryExpression : public ASTExpression
{
	BinaryOperator operation;
	ASTExpression* left, * right;

	ASTBinaryExpression(BinaryOperator operation, ASTExpression* left, ASTExpression* right) :
		operation(operation), left(left), right(right) { }

	IMPLEMENT_ACCEPT()
};

struct ASTCallExpression : public ASTExpression
{
	ASTExpression* expression;
	std::vector<ASTExpression*> args;

	ASTCallExpression(ASTExpression* expression, std::vector<ASTExpression*> const& args) :
		expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBoundCallExpression : public ASTExpression
{
	std::string identifier;
	std::vector<ASTExpression*> args;

	ASTBoundCallExpression(std::string identifier, std::vector<ASTExpression*> const& args) :
		identifier(identifier), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct ASTIndexExpression : public ASTExpression
{
	ASTExpression* expression;
	std::vector<ASTExpression*> args;

	ASTIndexExpression(ASTExpression* expression, std::vector<ASTExpression*> const& args) :
		expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBoundIndexExpression : public ASTExpression
{
	ASTExpression* expression;
	ASTExpression* index;

	ASTBoundIndexExpression(ASTExpression* expression, ASTExpression* index) :
		expression(expression), index(index) { }

	IMPLEMENT_ACCEPT()
};

struct ASTFieldExpression : public ASTExpression
{
	ASTExpression* expression;
	std::string fieldName;

	ASTFieldExpression(ASTExpression* expression, std::string const& fieldName) :
		expression(expression), fieldName(fieldName) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct ASTBlockStatement : public ASTStatement
{
	std::vector<ASTStatement*> statements;

	ASTBlockStatement(std::vector<ASTStatement*> statements) :
		statements(statements) { }

	IMPLEMENT_ACCEPT()
};

struct ASTExpressionStatement : public ASTStatement
{
	ASTExpression* expression;

	ASTExpressionStatement(ASTExpression* expression) :
		expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct ASTVariableStatement : public ASTStatement
{
	std::vector<std::pair<std::string, ASTExpression*>> items;

	ASTVariableStatement(std::vector<std::pair<std::string, ASTExpression*>> const& items) :
		items(items) { }

	IMPLEMENT_ACCEPT()
};

struct ASTReturnStatement : public ASTStatement
{
	ASTExpression* expression;

	ASTReturnStatement(ASTExpression* expression) :
		expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct ASTWhileStatement : public ASTStatement
{
	ASTExpression* condition;
	ASTStatement* body;

	ASTWhileStatement(ASTExpression* condition, ASTStatement* body) :
		condition(condition), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct ASTIfStatement : public ASTStatement
{
	ASTExpression* condition;
	ASTStatement* ifBody, * elseBody;

	ASTIfStatement(ASTExpression* condition, ASTStatement* ifBody, ASTStatement* elseBody) :
		condition(condition), ifBody(ifBody), elseBody(elseBody) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct ASTStructDeclaration : public ASTDeclaration
{
	std::string name;
	std::vector<std::pair<std::string, ASTType*>> fields;

	ASTStructDeclaration(std::string const& name, std::vector<std::pair<std::string, ASTType*>> const& fields) :
		name(name), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct ASTFunctionDeclaration : public ASTDeclaration
{
	std::string name;
	ASTType* result;
	std::vector<std::pair<std::string, ASTType*>> parameters;
	ASTStatement* body;

	ASTFunctionDeclaration(std::string const& name, ASTType* result, std::vector<std::pair<std::string, ASTType*>> const& parameters, ASTStatement* body) :
		name(name), result(result), parameters(parameters), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct ASTExternFunctionDeclaration : public ASTDeclaration
{
	std::string lib;
	std::string name;
	ASTType* result;
	std::vector<std::pair<std::string, ASTType*>> parameters;

	ASTExternFunctionDeclaration(std::string const& lib, std::string const& name, ASTType* result, std::vector<std::pair<std::string, ASTType*>> const& parameters) :
		lib(lib), name(name), result(result), parameters(parameters) { }

	IMPLEMENT_ACCEPT()
};

///////////////////////////////////////////

struct ASTSourceFile : public ASTNode
{
	std::vector<std::string> modulePath;
	std::vector<std::vector<std::string>> importPaths;
	std::vector<ASTDeclaration*> declarations;

	ASTSourceFile(std::vector<std::string> const& modulePath, std::vector<std::vector<std::string>> const& importPaths, std::vector<ASTDeclaration*> const& declarations) :
		modulePath(modulePath), importPaths(importPaths), declarations(declarations) { }

	IMPLEMENT_ACCEPT()
};