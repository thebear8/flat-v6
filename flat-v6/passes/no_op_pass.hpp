#pragma once
#include "../data/ast.hpp"

class NoOpPass : public Visitor<AstNode*>
{
	virtual AstNode* visit(IntegerExpression* node) override;
	virtual AstNode* visit(BoolExpression* node) override;
	virtual AstNode* visit(CharExpression* node) override;
	virtual AstNode* visit(StringExpression* node) override;
	virtual AstNode* visit(IdentifierExpression* node) override;
	virtual AstNode* visit(StructExpression* node) override;
	virtual AstNode* visit(UnaryExpression* node) override;
	virtual AstNode* visit(BinaryExpression* node) override;
	virtual AstNode* visit(CallExpression* node) override;
	virtual AstNode* visit(BoundCallExpression* node) override;
	virtual AstNode* visit(IndexExpression* node) override;
	virtual AstNode* visit(BoundIndexExpression* node) override;
	virtual AstNode* visit(FieldExpression* node) override;

	virtual AstNode* visit(BlockStatement* node) override;
	virtual AstNode* visit(ExpressionStatement* node) override;
	virtual AstNode* visit(VariableStatement* node) override;
	virtual AstNode* visit(ReturnStatement* node) override;
	virtual AstNode* visit(WhileStatement* node) override;
	virtual AstNode* visit(IfStatement* node) override;

	virtual AstNode* visit(StructDeclaration* node) override;
	virtual AstNode* visit(FunctionDeclaration* node) override;
	virtual AstNode* visit(ExternFunctionDeclaration* node) override;
	virtual AstNode* visit(Module* node) override;

	virtual AstNode* visit(AstNode* node) override { throw std::exception(); }
	virtual AstNode* visit(Expression* node) override { throw std::exception(); }
	virtual AstNode* visit(Statement* node) override { throw std::exception(); }
	virtual AstNode* visit(Declaration* node) override { throw std::exception(); }
};