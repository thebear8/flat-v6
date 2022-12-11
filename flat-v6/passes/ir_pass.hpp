#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"

class IRPass : public ASTVisitor<IRNode*>
{
private:
	GraphContext& irCtx;

	virtual IRNode* visit(ASTIntegerExpression* node) override;
	virtual IRNode* visit(ASTBoolExpression* node) override;
	virtual IRNode* visit(ASTCharExpression* node) override;
	virtual IRNode* visit(ASTStringExpression* node) override;
	virtual IRNode* visit(ASTIdentifierExpression* node) override;
	virtual IRNode* visit(ASTStructExpression* node) override;
	virtual IRNode* visit(ASTUnaryExpression* node) override;
	virtual IRNode* visit(ASTBinaryExpression* node) override;
	virtual IRNode* visit(ASTCallExpression* node) override;
	virtual IRNode* visit(ASTBoundCallExpression* node) override;
	virtual IRNode* visit(ASTIndexExpression* node) override;
	virtual IRNode* visit(ASTBoundIndexExpression* node) override;
	virtual IRNode* visit(ASTFieldExpression* node) override;

	virtual IRNode* visit(ASTBlockStatement* node) override;
	virtual IRNode* visit(ASTExpressionStatement* node) override;
	virtual IRNode* visit(ASTVariableStatement* node) override;
	virtual IRNode* visit(ASTReturnStatement* node) override;
	virtual IRNode* visit(ASTWhileStatement* node) override;
	virtual IRNode* visit(ASTIfStatement* node) override;

	virtual IRNode* visit(ASTStructDeclaration* node) override;
	virtual IRNode* visit(ASTFunctionDeclaration* node) override;
	virtual IRNode* visit(ASTExternFunctionDeclaration* node) override;
	virtual IRNode* visit(ASTSourceFile* node) override;
};