#pragma once
#include "../../ast/ast.hpp"
#include "../support/ast_type_resolver.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class Environment;

class ConstraintExtractionPass : ASTVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ASTTypeResolver& m_resolver;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    ConstraintExtractionPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        ASTTypeResolver& resolver
    )
        : m_logger(logger), m_compCtx(compCtx), m_resolver(resolver)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    void visit(ASTStructDeclaration* node) override {}
    void visit(ASTConstraintDeclaration* node) override;
    void visit(ASTFunctionDeclaration* node) override {}
    void visit(ASTSourceFile* node) override;
};