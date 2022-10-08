#pragma once
#include "../data/ast.hpp"

class NoOpPass : public Visitor<void>
{
	virtual void visit(IntegerExpression* node) override;
	virtual void visit(BoolExpression* node) override;
	virtual void visit(CharExpression* node) override;
	virtual void visit(StringExpression* node) override;
	virtual void visit(IdentifierExpression* node) override;
	virtual void visit(StructExpression* node) override;
	virtual void visit(UnaryExpression* node) override;
	virtual void visit(BinaryExpression* node) override;
	virtual void visit(CallExpression* node) override;
	virtual void visit(BoundCallExpression* node) override;
	virtual void visit(IndexExpression* node) override;
	virtual void visit(BoundIndexExpression* node) override;
	virtual void visit(FieldExpression* node) override;

	virtual void visit(BlockStatement* node) override;
	virtual void visit(ExpressionStatement* node) override;
	virtual void visit(VariableStatement* node) override;
	virtual void visit(ReturnStatement* node) override;
	virtual void visit(WhileStatement* node) override;
	virtual void visit(IfStatement* node) override;

	virtual void visit(StructDeclaration* node) override;
	virtual void visit(FunctionDeclaration* node) override;
	virtual void visit(ExternFunctionDeclaration* node) override;
	virtual void visit(Module* node) override;

	virtual void visit(AstNode* node) override { throw std::exception(); }
	virtual void visit(Expression* node) override { throw std::exception(); }
	virtual void visit(Statement* node) override { throw std::exception(); }
	virtual void visit(Declaration* node) override { throw std::exception(); }
};