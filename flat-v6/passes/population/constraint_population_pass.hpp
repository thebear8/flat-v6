#pragma once
#include "../../ast/ast.hpp"

class IRNode;
class ErrorLogger;
class CompilationContext;
class GraphContext;
class ASTTypeResolver;
class ConstraintInstantiator;
class Environment;

class ConstraintPopulationPass : ASTVisitor<IRNode*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_envCtx;
    ASTTypeResolver& m_resolver;
    ConstraintInstantiator& m_constraintInstantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    ConstraintPopulationPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        GraphContext& envCtx,
        ASTTypeResolver& resolver,
        ConstraintInstantiator& constraintInstantiator
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_envCtx(envCtx),
          m_resolver(resolver),
          m_constraintInstantiator(constraintInstantiator)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    IRNode* visit(ASTRequirement* node) override;
    IRNode* visit(ASTConstraintCondition* node) override;
    IRNode* visit(ASTStructDeclaration* node) override { return nullptr; }
    IRNode* visit(ASTConstraintDeclaration* node) override;
    IRNode* visit(ASTFunctionDeclaration* node) override { return nullptr; }
    IRNode* visit(ASTSourceFile* node) override;
};