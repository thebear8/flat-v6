#pragma once
#include "../ast/ast.hpp"
#include "../compiler.hpp"

class ModuleExtractionPass : ASTVisitor<ModuleContext*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_irCtx;

public:
    ModuleExtractionPass(
        ErrorLogger& logger, CompilationContext& compCtx, GraphContext& memCtx
    )
        : m_logger(logger), m_compCtx(compCtx), m_irCtx(memCtx)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

public:
    virtual ModuleContext* visit(ASTSourceFile* node) override;
};