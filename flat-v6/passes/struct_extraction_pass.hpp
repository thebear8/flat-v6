#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"
#include "../compiler.hpp"

class StructExtractionPass : ASTVisitor<void>
{
private:
	ErrorLogger& logger;
	CompilationContext& compCtx;
	ModuleContext& modCtx;

public:
	StructExtractionPass(ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx) :
		logger(logger), compCtx(compCtx), modCtx(modCtx) { }

public:
	void process(ASTSourceFile* sourceFile);

private:
	virtual void visit(ASTStructDeclaration* node) override;
	virtual void visit(ASTFunctionDeclaration* node) override { }
	virtual void visit(ASTExternFunctionDeclaration* node) override { }
	virtual void visit(ASTSourceFile* node) override;
};