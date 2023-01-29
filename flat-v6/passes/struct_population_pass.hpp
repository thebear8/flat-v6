#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"

class StructPopulationPass : ASTVisitor<void>
{
private:
    ErrorLogger& logger;
    CompilationContext& compCtx;
    ModuleContext& modCtx;

public:
    StructPopulationPass(
        ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx)
        : logger(logger), compCtx(compCtx), modCtx(modCtx)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    virtual void visit(ASTStructDefinition* node) override;
    virtual void visit(ASTFunctionDefinition* node) override {}
    virtual void visit(ASTExternFunctionDefinition* node) override {}
    virtual void visit(ASTSourceFile* node) override;
};