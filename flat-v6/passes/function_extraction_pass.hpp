#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"

class FunctionExtractionPass : IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ModuleContext& m_modCtx;

public:
    FunctionExtractionPass(
        ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx
    )
        : m_logger(logger), m_compCtx(compCtx), m_modCtx(modCtx)
    {
    }

public:
    void process(IRSourceFile* sourceFile);

private:
    virtual void visit(IRConstraintDeclaration* node) override {}
    virtual void visit(IRStructDeclaration* node) override {}
    virtual void visit(IRFunctionDeclaration* node) override;
    virtual void visit(IRSourceFile* node) override;
};