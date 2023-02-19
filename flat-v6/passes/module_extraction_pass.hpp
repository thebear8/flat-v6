#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"

class ModuleExtractionPass : ASTVisitor<ModuleContext*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_memCtx;

public:
    ModuleExtractionPass(
        ErrorLogger& logger, CompilationContext& compCtx, GraphContext& memCtx
    )
        : m_logger(logger), m_compCtx(compCtx), m_memCtx(memCtx)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

public:
    virtual ModuleContext* visit(ASTSourceFile* node) override;
};