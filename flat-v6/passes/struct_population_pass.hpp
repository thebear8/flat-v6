#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"
#include "../compiler.hpp"

class StructPopulationPass : public ASTVisitor<void>
{
private:
	ErrorLogger& logger;
	GraphContext& irCtx;
	CompilationContext& compilationCtx;
	ModuleContext* moduleCtx;

	virtual void visit(ASTStructDeclaration* node) override;
	virtual void visit(ASTFunctionDeclaration* node) override { }
	virtual void visit(ASTExternFunctionDeclaration* node) override { }
	virtual void visit(ASTSourceFile* node) override;
};