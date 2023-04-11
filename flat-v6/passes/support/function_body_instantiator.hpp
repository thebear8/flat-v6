#pragma once
#include "../../ir/ir.hpp"

class Environment;
class GraphContext;
class StructInstantiator;
class FunctionInstantiator;
class CallTargetResolver;

class FunctionBodyInstantiator : IRVisitor<IRNode*>
{
private:
    GraphContext& m_envCtx;
    StructInstantiator& m_structInstantiator;
    FunctionInstantiator& m_functionInstantiator;
    CallTargetResolver& m_callTargetResolver;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    FunctionBodyInstantiator(
        GraphContext& envCtx,
        StructInstantiator& structInstantiator,
        FunctionInstantiator& functionInstantiator,
        CallTargetResolver& callTargetResolver
    )
        : m_envCtx(envCtx),
          m_structInstantiator(structInstantiator),
          m_functionInstantiator(functionInstantiator),
          m_callTargetResolver(callTargetResolver)
    {
    }

    /// @brief Fully instantiate the body of a function template
    /// @param functionInstantiation The function instantiation of which to
    /// update the body
    /// @return The updated function instantiation
    IRFunctionInstantiation* updateBody(
        IRFunctionInstantiation* functionInstantiation
    );

private:
    IRNode* visit(IRIntegerExpression* node) override { return node; }
    IRNode* visit(IRBoolExpression* node) override { return node; }
    IRNode* visit(IRCharExpression* node) override { return node; }
    IRNode* visit(IRStringExpression* node) override { return node; }
    IRNode* visit(IRIdentifierExpression* node) override;
    IRNode* visit(IRStructExpression* node) override;
    IRNode* visit(IRUnaryExpression* node) override;
    IRNode* visit(IRBinaryExpression* node) override;
    IRNode* visit(IRCallExpression* node) override;
    IRNode* visit(IRIndexExpression* node) override;
    IRNode* visit(IRFieldExpression* node) override;

    IRNode* visit(IRBlockStatement* node) override;
    IRNode* visit(IRExpressionStatement* node) override;
    IRNode* visit(IRVariableStatement* node) override;
    IRNode* visit(IRReturnStatement* node) override;
    IRNode* visit(IRWhileStatement* node) override;
    IRNode* visit(IRIfStatement* node) override;

    IRNode* visit(IRFunctionHead* node) override;
    IRNode* visit(IRStructInstantiation* node) override;
    IRNode* visit(IRFunctionInstantiation* node) override;

    IRNode* visit(IRGenericType* node) override;
    IRNode* visit(IRVoidType* node) override { return node; }
    IRNode* visit(IRBoolType* node) override { return node; }
    IRNode* visit(IRIntegerType* node) override { return node; }
    IRNode* visit(IRCharType* node) override { return node; }
    IRNode* visit(IRStringType* node) override { return node; }
    IRNode* visit(IRPointerType* node) override;
    IRNode* visit(IRArrayType* node) override;
};