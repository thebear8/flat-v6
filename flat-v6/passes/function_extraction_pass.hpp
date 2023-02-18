#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"

class FunctionExtractionPass : IRVisitor<void> {
private:
    ErrorLogger& logger;
    CompilationContext& compCtx;
    ModuleContext& modCtx;

public:
    FunctionExtractionPass(
        ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx
    )
        : logger(logger), compCtx(compCtx), modCtx(modCtx) {}

public:
    void process(IRSourceFile* sourceFile);

private:
    virtual void visit(IRConstraintDeclaration* node) override {}
    virtual void visit(IRStructDeclaration* node) override {}
    virtual void visit(IRFunctionDeclaration* node) override;
    virtual void visit(IRSourceFile* node) override;
};