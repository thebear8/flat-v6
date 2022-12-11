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
	std::unordered_map<std::string, ASTStructDeclaration*> structs;
	std::unordered_multimap<std::string, ASTFunctionDeclaration*> functions;
	std::unordered_multimap<std::string, ASTExternFunctionDeclaration*> externFunctions;
	std::unordered_map<std::string, Type*> localVariables;

public:
	SemanticPass(ErrorLogger& logger, AstContext& astCtx, TypeContext& typeCtx) :
		logger(logger), astCtx(astCtx), typeCtx(typeCtx), functionResult(nullptr), expectedFunctionResult(nullptr) { }

public:
	void analyze(ASTNode* program);

protected:
	Type* getFunctionResult(std::string const& name, std::vector<Type*> const& args);
	Type* getFunctionResult(std::string const& name, std::vector<Type*> const& args, ASTNode* current);

protected:
	virtual Type* visit(ASTIntegerExpression* node) override;
	virtual Type* visit(ASTBoolExpression* node) override;
	virtual Type* visit(ASTCharExpression* node) override;
	virtual Type* visit(ASTStringExpression* node) override;
	virtual Type* visit(ASTIdentifierExpression* node) override;
	virtual Type* visit(ASTStructExpression* node) override;
	virtual Type* visit(ASTUnaryExpression* node) override;
	virtual Type* visit(ASTBinaryExpression* node) override;
	virtual Type* visit(ASTCallExpression* node) override;
	virtual Type* visit(ASTIndexExpression* node) override;
	virtual Type* visit(ASTFieldExpression* node) override;

	virtual Type* visit(ASTBlockStatement* node) override;
	virtual Type* visit(ASTExpressionStatement* node) override;
	virtual Type* visit(ASTVariableStatement* node) override;
	virtual Type* visit(ASTReturnStatement* node) override;
	virtual Type* visit(ASTWhileStatement* node) override;
	virtual Type* visit(ASTIfStatement* node) override;

	virtual Type* visit(ASTStructDeclaration* node) override;
	virtual Type* visit(ASTFunctionDeclaration* node) override;
	virtual Type* visit(ASTExternFunctionDeclaration* node) override;
	virtual Type* visit(ASTSourceFile* node) override;
};