#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"
#include "../compiler.hpp"

class StructExtractionPass : public ASTVisitor<void>
{
private:
	ErrorLogger& logger;
	GraphContext& irCtx;
	CompilationContext& compilationCtx;
	ModuleContext* moduleCtx;

	virtual void visit(ASTIntegerExpression* node) override { }
	virtual void visit(ASTBoolExpression* node) override { }
	virtual void visit(ASTCharExpression* node) override { }
	virtual void visit(ASTStringExpression* node) override { }
	virtual void visit(ASTIdentifierExpression* node) override { }
	virtual void visit(ASTStructExpression* node) override { }
	virtual void visit(ASTUnaryExpression* node) override { }
	virtual void visit(ASTBinaryExpression* node) override { }
	virtual void visit(ASTCallExpression* node) override { }
	virtual void visit(ASTBoundCallExpression* node) override { }
	virtual void visit(ASTIndexExpression* node) override { }
	virtual void visit(ASTBoundIndexExpression* node) override { }
	virtual void visit(ASTFieldExpression* node) override { }

	virtual void visit(ASTBlockStatement* node) override { }
	virtual void visit(ASTExpressionStatement* node) override { }
	virtual void visit(ASTVariableStatement* node) override { }
	virtual void visit(ASTReturnStatement* node) override { }
	virtual void visit(ASTWhileStatement* node) override { }
	virtual void visit(ASTIfStatement* node) override { }

	virtual void visit(ASTStructDeclaration* node) override;
	virtual void visit(ASTFunctionDeclaration* node) override { }
	virtual void visit(ASTExternFunctionDeclaration* node) override { }
	virtual void visit(ASTSourceFile* node) override;
};