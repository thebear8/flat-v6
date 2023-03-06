#pragma once
#include "../ast/ast.hpp"
#include "support/ast_type_resolver.hpp"
#include "support/instantiator.hpp"

class IRNode;
class ErrorLogger;
class CompilationContext;
class GraphContext;
class Environment;

class ConstraintPopulationPass : ASTVisitor<IRNode*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ASTTypeResolver m_resolver;
    Instantiator m_instantiator;

    IRModule* m_module;
    GraphContext* m_irCtx;
    Environment* m_env;

public:
    ConstraintPopulationPass(ErrorLogger& logger, CompilationContext& compCtx)
        : m_logger(logger),
          m_compCtx(compCtx),
          m_module(nullptr),
          m_env(nullptr)
    {
    }

public:
    void process(ASTSourceFile* sourceFile);

private:
    IRNode* visit(ASTRequirement* node) override;
    IRNode* visit(ASTConstraintCondition* node) override;
    IRNode* visit(ASTStructDeclaration* node) override {}
    IRNode* visit(ASTConstraintDeclaration* node) override;
    IRNode* visit(ASTFunctionDeclaration* node) override {}
    IRNode* visit(ASTSourceFile* node) override;
};