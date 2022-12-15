#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"
#include "../compiler.hpp"

class FunctionExtractionPass : public IRVisitor<void>
{
private:
	ErrorLogger& logger;
	CompilationContext& compCtx;
	ModuleContext& modCtx;

public:
	FunctionExtractionPass(ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx) :
		logger(logger), compCtx(compCtx), modCtx(modCtx) {}

	virtual void visit(IRStructDeclaration* node) override {}
	virtual void visit(IRFunctionDeclaration* node) override;
	virtual void visit(IRSourceFile* node) override;
};