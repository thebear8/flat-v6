#pragma once
#include <vector>
#include <string>
#include <unordered_map>

#include "token.hpp"
#include "operator.hpp"
#include "source_ref.hpp"
#include "../util/visitor.hpp"

using ASTTripleDispatchVisitor = triple_dispatch_visitor::TripleDispatchVisitor<
	struct ASTNode,
	struct ASTType,
	struct ASTExpression,
	struct ASTStatement,
	struct ASTDeclaration,

	struct ASTNamedType,
	struct ASTPointerType,
	struct ASTArrayType,

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

struct ASTNode : ASTTripleDispatchVisitor::NodeBase
{
	SourceRef location;

	ASTNode(SourceRef const& location) :
		location(location) {}

	IMPLEMENT_ACCEPT()
};

struct ASTType : public ASTNode
{
	ASTType(SourceRef const& location) :
		ASTNode(location) { }

	IMPLEMENT_ACCEPT()
};

struct ASTExpression : public ASTNode
{
	ASTType* type;

	ASTExpression(SourceRef const& location) :
		ASTNode(location), type(nullptr) { }

	IMPLEMENT_ACCEPT()
};

struct ASTStatement : public ASTNode
{
	ASTStatement(SourceRef const& location) :
		ASTNode(location) { }

	IMPLEMENT_ACCEPT()
};

struct ASTDeclaration : public ASTNode
{
	ASTDeclaration(SourceRef const& location) :
		ASTNode(location) { }

	IMPLEMENT_ACCEPT()
};

//

struct ASTNamedType : public ASTType
{
	std::string name;

	ASTNamedType(SourceRef const& location, std::string const& name) :
		ASTType(location), name(name) { }

	IMPLEMENT_ACCEPT()
};

struct ASTPointerType : public ASTType
{
	ASTType* base;

	ASTPointerType(SourceRef const& location, ASTType* base) :
		ASTType(location), base(base) { }

	IMPLEMENT_ACCEPT()
};

struct ASTArrayType : public ASTType
{
	ASTType* base;

	ASTArrayType(SourceRef const& location, ASTType* base) :
		ASTType(location), base(base) { }

	IMPLEMENT_ACCEPT()
};

//

struct ASTIntegerExpression : public ASTExpression
{
	std::string value;
	std::string suffix;

	ASTIntegerExpression(SourceRef const& location, std::string const& value, std::string const& suffix) :
		ASTExpression(location), value(value), suffix(suffix) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBoolExpression : public ASTExpression
{
	std::string value;

	ASTBoolExpression(SourceRef const& location, std::string const& value) :
		ASTExpression(location), value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTCharExpression : public ASTExpression
{
	std::string value;

	ASTCharExpression(SourceRef const& location, std::string const& value) :
		ASTExpression(location), value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTStringExpression : public ASTExpression
{
	std::string value;

	ASTStringExpression(SourceRef const& location, std::string const& value) :
		ASTExpression(location), value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTIdentifierExpression : public ASTExpression
{
	std::string value;

	ASTIdentifierExpression(SourceRef const& location, std::string const& value) :
		ASTExpression(location), value(value) { }

	IMPLEMENT_ACCEPT()
};

struct ASTStructExpression : public ASTExpression
{
	std::string structName;
	std::vector<std::pair<std::string, ASTExpression*>> fields;

	ASTStructExpression(SourceRef const& location, std::string const& structName, std::vector<std::pair<std::string, ASTExpression*>> const& fields) :
		ASTExpression(location), structName(structName), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct ASTUnaryExpression : public ASTExpression
{
	UnaryOperator operation;
	ASTExpression* expression;

	ASTUnaryExpression(SourceRef const& location, UnaryOperator operation, ASTExpression* expression) :
		ASTExpression(location), operation(operation), expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBinaryExpression : public ASTExpression
{
	BinaryOperator operation;
	ASTExpression* left, * right;

	ASTBinaryExpression(SourceRef const& location, BinaryOperator operation, ASTExpression* left, ASTExpression* right) :
		ASTExpression(location), operation(operation), left(left), right(right) { }

	IMPLEMENT_ACCEPT()
};

struct ASTCallExpression : public ASTExpression
{
	ASTExpression* expression;
	std::vector<ASTExpression*> args;

	ASTCallExpression(SourceRef const& location, ASTExpression* expression, std::vector<ASTExpression*> const& args) :
		ASTExpression(location), expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBoundCallExpression : public ASTExpression
{
	std::string identifier;
	std::vector<ASTExpression*> args;

	ASTBoundCallExpression(SourceRef const& location, std::string identifier, std::vector<ASTExpression*> const& args) :
		ASTExpression(location), identifier(identifier), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct ASTIndexExpression : public ASTExpression
{
	ASTExpression* expression;
	std::vector<ASTExpression*> args;

	ASTIndexExpression(SourceRef const& location, ASTExpression* expression, std::vector<ASTExpression*> const& args) :
		ASTExpression(location), expression(expression), args(args) { }

	IMPLEMENT_ACCEPT()
};

struct ASTBoundIndexExpression : public ASTExpression
{
	ASTExpression* expression;
	ASTExpression* index;

	ASTBoundIndexExpression(SourceRef const& location, ASTExpression* expression, ASTExpression* index) :
		ASTExpression(location), expression(expression), index(index) { }

	IMPLEMENT_ACCEPT()
};

struct ASTFieldExpression : public ASTExpression
{
	ASTExpression* expression;
	std::string fieldName;

	ASTFieldExpression(SourceRef const& location, ASTExpression* expression, std::string const& fieldName) :
		ASTExpression(location), expression(expression), fieldName(fieldName) { }

	IMPLEMENT_ACCEPT()
};

//

struct ASTBlockStatement : public ASTStatement
{
	std::vector<ASTStatement*> statements;

	ASTBlockStatement(SourceRef const& location, std::vector<ASTStatement*> statements) :
		ASTStatement(location), statements(statements) { }

	IMPLEMENT_ACCEPT()
};

struct ASTExpressionStatement : public ASTStatement
{
	ASTExpression* expression;

	ASTExpressionStatement(SourceRef const& location, ASTExpression* expression) :
		ASTStatement(location), expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct ASTVariableStatement : public ASTStatement
{
	std::vector<std::pair<std::string, ASTExpression*>> items;

	ASTVariableStatement(SourceRef const& location, std::vector<std::pair<std::string, ASTExpression*>> const& items) :
		ASTStatement(location), items(items) { }

	IMPLEMENT_ACCEPT()
};

struct ASTReturnStatement : public ASTStatement
{
	ASTExpression* expression;

	ASTReturnStatement(SourceRef const& location, ASTExpression* expression) :
		ASTStatement(location), expression(expression) { }

	IMPLEMENT_ACCEPT()
};

struct ASTWhileStatement : public ASTStatement
{
	ASTExpression* condition;
	ASTStatement* body;

	ASTWhileStatement(SourceRef const& location, ASTExpression* condition, ASTStatement* body) :
		ASTStatement(location), condition(condition), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct ASTIfStatement : public ASTStatement
{
	ASTExpression* condition;
	ASTStatement* ifBody, * elseBody;

	ASTIfStatement(SourceRef const& location, ASTExpression* condition, ASTStatement* ifBody, ASTStatement* elseBody) :
		ASTStatement(location), condition(condition), ifBody(ifBody), elseBody(elseBody) { }

	IMPLEMENT_ACCEPT()
};

//

struct ASTStructDeclaration : public ASTDeclaration
{
	std::string name;
	std::vector<std::pair<std::string, ASTType*>> fields;

	ASTStructDeclaration(SourceRef const& location, std::string const& name, std::vector<std::pair<std::string, ASTType*>> const& fields) :
		ASTDeclaration(location), name(name), fields(fields) { }

	IMPLEMENT_ACCEPT()
};

struct ASTFunctionDeclaration : public ASTDeclaration
{
	std::string name;
	ASTType* result;
	std::vector<std::pair<std::string, ASTType*>> parameters;
	ASTStatement* body;

	ASTFunctionDeclaration(SourceRef const& location, std::string const& name, ASTType* result, std::vector<std::pair<std::string, ASTType*>> const& parameters, ASTStatement* body) :
		ASTDeclaration(location), name(name), result(result), parameters(parameters), body(body) { }

	IMPLEMENT_ACCEPT()
};

struct ASTExternFunctionDeclaration : public ASTDeclaration
{
	std::string lib;
	std::string name;
	ASTType* result;
	std::vector<std::pair<std::string, ASTType*>> parameters;

	ASTExternFunctionDeclaration(SourceRef const& location, std::string const& lib, std::string const& name, ASTType* result, std::vector<std::pair<std::string, ASTType*>> const& parameters) :
		ASTDeclaration(location), lib(lib), name(name), result(result), parameters(parameters) { }

	IMPLEMENT_ACCEPT()
};

//

struct ASTSourceFile : public ASTNode
{
	std::string modulePath;
	std::vector<std::string> importPaths;
	std::vector<ASTDeclaration*> declarations;

	ASTSourceFile(SourceRef const& location, std::string const& modulePath, std::vector<std::string> const& importPaths, std::vector<ASTDeclaration*> const& declarations) :
		ASTNode(location), modulePath(modulePath), importPaths(importPaths), declarations(declarations) { }

	IMPLEMENT_ACCEPT()
};