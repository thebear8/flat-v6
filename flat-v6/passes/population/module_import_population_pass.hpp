#pragma once
#include "../../ast/ast.hpp"

class ErrorLogger;
class CompilationContext;

class ModuleImportPopulationPass : ASTVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;

public:
    ModuleImportPopulationPass(ErrorLogger& logger, CompilationContext& compCtx)
        : m_logger(logger), m_compCtx(compCtx)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

public:
    virtual void visit(ASTSourceFile* node) override;
};