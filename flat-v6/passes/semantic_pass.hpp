#pragma once
#include <string>
#include <string_view>
#include <ostream>
#include <unordered_map>

#include "../data/ast.hpp"
#include "../type/type.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"

class SemanticPass : protected Visitor<Type*>
{
private:
	ErrorLogger& logger;
	AstContext& astCtx;
	TypeContext& typeCtx;
	Type* functionResult, * expectedFunctionResult;
	std::unordered_map<std::string, StructDeclaration*> structs;
	std::unordered_multimap<std::string, FunctionDeclaration*> functions;
	std::unordered_multimap<std::string, ExternFunctionDeclaration*> externFunctions;
	std::unordered_map<std::string, Type*> localVariables;

public:
	SemanticPass(ErrorLogger& logger, AstContext& astCtx, TypeContext& typeCtx) :
		logger(logger), astCtx(astCtx), typeCtx(typeCtx), functionResult(nullptr), expectedFunctionResult(nullptr) { }

public:
	void analyze(AstNode* program);

protected:
	Type* getFunctionResult(std::string const& name, std::vector<Type*> const& args);
	Type* getFunctionResult(std::string const& name, std::vector<Type*> const& args, AstNode* current);

protected:
	virtual Type* visit(IntegerExpression* node) override;
	virtual Type* visit(BoolExpression* node) override;
	virtual Type* visit(CharExpression* node) override;
	virtual Type* visit(StringExpression* node) override;
	virtual Type* visit(IdentifierExpression* node) override;
	virtual Type* visit(StructExpression* node) override;
	virtual Type* visit(UnaryExpression* node) override;
	virtual Type* visit(BinaryExpression* node) override;
	virtual Type* visit(CallExpression* node) override;
	virtual Type* visit(IndexExpression* node) override;
	virtual Type* visit(FieldExpression* node) override;

	virtual Type* visit(BlockStatement* node) override;
	virtual Type* visit(ExpressionStatement* node) override;
	virtual Type* visit(VariableStatement* node) override;
	virtual Type* visit(ReturnStatement* node) override;
	virtual Type* visit(WhileStatement* node) override;
	virtual Type* visit(IfStatement* node) override;

	virtual Type* visit(StructDeclaration* node) override;
	virtual Type* visit(FunctionDeclaration* node) override;
	virtual Type* visit(ExternFunctionDeclaration* node) override;
	virtual Type* visit(Module* node) override;

	virtual Type* visit(AstNode* node) override { return dispatch(node); }
	virtual Type* visit(Expression* node) override { return dispatch(node); }
	virtual Type* visit(Statement* node) override { return dispatch(node); }
	virtual Type* visit(Declaration* node) override { return dispatch(node); }
	virtual Type* visit(BoundCallExpression* node) override { throw std::exception(); }
	virtual Type* visit(BoundIndexExpression* node) override { throw std::exception(); }
};