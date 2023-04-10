#pragma once
#include "../../ir/ir.hpp"

class Environment;
class GraphContext;
class StructInstantiator;
class ConstraintInstantiator;

class FunctionInstantiator : IRVisitor<IRNode*>
{
private:
    GraphContext& m_envCtx;
    StructInstantiator& m_structInstantiator;
    ConstraintInstantiator& m_constraintInstantiator;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    FunctionInstantiator(
        GraphContext& envCtx,
        StructInstantiator& structInstantiator,
        ConstraintInstantiator& constraintInstantiator
    )
        : m_envCtx(envCtx),
          m_structInstantiator(structInstantiator),
          m_constraintInstantiator(constraintInstantiator)
    {
    }

    /// @brief Get the instantiation of the given function with the given type
    /// args. If this instantiation does not yet exist, create it and add it to
    /// the parent module of the function template.
    /// @param functionTemplate The function template to get an instantiation
    /// of
    /// @param typeArgs The type args of the instantiation
    /// @return The function instantiation
    IRFunctionInstantiation* getFunctionInstantiation(
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Fully instantiate a function template
    /// @param functionInstantiation The function instantiation to update
    /// @return The updated function instantiation
    IRFunctionInstantiation* updateFunctionInstantiation(
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
    IRNode* visit(IRConstraintInstantiation* node) override;
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