#pragma once
#include "../../ast/ast.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class Environment;
class ASTTypeResolver;
class IRModule;

class FunctionExtractionPass : ASTVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_envCtx;
    ASTTypeResolver& m_resolver;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    FunctionExtractionPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        GraphContext& envCtx,
        ASTTypeResolver& resolver
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_envCtx(envCtx),
          m_resolver(resolver)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    void visit(ASTStructDeclaration* node) override {}
    void visit(ASTConstraintDeclaration* node) override {}
    void visit(ASTFunctionDeclaration* node) override;
    void visit(ASTSourceFile* node) override;
};