#pragma once
#include "../ast/ast.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class Environment;

class StructPopulationPass : ASTVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;

    IRModule* m_module;
    GraphContext* m_irCtx;
    Environment* m_env;

public:
    StructPopulationPass(ErrorLogger& logger, CompilationContext& compCtx)
        : m_logger(logger),
          m_compCtx(compCtx),
          m_module(nullptr),
          m_env(nullptr)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
public:
    void process(ASTSourceFile* sourceFile);

private:
    void visit(ASTStructDeclaration* node) override;
    void visit(ASTConstraintDeclaration* node) override {}
    void visit(ASTFunctionDeclaration* node) override {}
    void visit(ASTSourceFile* node) override;
};