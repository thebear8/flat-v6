#pragma once
#include "../../ast/ast.hpp"
#include "../../compiler.hpp"
#include "../../ir/ir.hpp"

class StructExtractionPass : ASTVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;

    IRModule* m_module;
    GraphContext* m_irCtx;
    Environment* m_env;

public:
    StructExtractionPass(ErrorLogger& logger, CompilationContext& compCtx)
        : m_logger(logger),
          m_compCtx(compCtx),
          m_module(nullptr),
          m_env(nullptr)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    virtual void visit(ASTStructDeclaration* node) override;
    virtual void visit(ASTConstraintDeclaration* node) override {}
    virtual void visit(ASTFunctionDeclaration* node) override {}
    virtual void visit(ASTSourceFile* node) override;
};