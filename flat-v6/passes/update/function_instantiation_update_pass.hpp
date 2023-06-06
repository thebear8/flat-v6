#pragma once
#include "../../ir/ir.hpp"

class GraphContext;
class Instantiator;
class CallTargetResolver;
class Environment;

class FunctionInstantiationUpdatePass : IRVisitor<IRNode*>
{
private:
    GraphContext& m_envCtx;
    Instantiator& m_instantiator;
    CallTargetResolver& m_callTargetResolver;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    FunctionInstantiationUpdatePass(
        GraphContext& envCtx,
        Instantiator& instantiator,
        CallTargetResolver& callTargetResolver
    )
        : m_envCtx(envCtx),
          m_instantiator(instantiator),
          m_callTargetResolver(callTargetResolver)
    {
    }

public:
    void process(IRModule* node);
    IRNormalFunction* update(IRNormalFunction* node);

private:
    IRNode* visit(IRIntegerExpression* node) override { return node; }
    IRNode* visit(IRBoolExpression* node) override { return node; }
    IRNode* visit(IRCharExpression* node) override { return node; }
    IRNode* visit(IRStringExpression* node) override { return node; }
    IRNode* visit(IRIdentifierExpression* node) override;
    IRNode* visit(IRStructExpression* node) override;
    IRNode* visit(IRBoundCallExpression* node) override;
    IRNode* visit(IRFieldExpression* node) override;

    IRNode* visit(IRBlockStatement* node) override;
    IRNode* visit(IRExpressionStatement* node) override;
    IRNode* visit(IRVariableStatement* node) override;
    IRNode* visit(IRReturnStatement* node) override;
    IRNode* visit(IRWhileStatement* node) override;
    IRNode* visit(IRIfStatement* node) override;

    IRNode* visit(IRConstraintFunction* node) override;
    IRNode* visit(IRIntrinsicFunction* node) override;
    IRNode* visit(IRNormalFunction* node) override;
};