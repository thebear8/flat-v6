#pragma once
#include "../../ir/ir.hpp"

class Environment;
class GraphContext;
class StructInstantiator;

class ConstraintInstantiator : IRVisitor<IRNode*>
{
private:
    GraphContext& m_envCtx;
    StructInstantiator& m_structInstantiator;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    ConstraintInstantiator(
        GraphContext& envCtx, StructInstantiator& structInstantiator
    )
        : m_envCtx(envCtx), m_structInstantiator(structInstantiator)
    {
    }

    /// @brief Get the instantiation of the given constraint with the given type
    /// args. If this instantiation does not yet exist, create it and add it to
    /// the parent module of the constraint template.
    /// @param constraintTemplate The constraint template to create an
    /// instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The constraint instantiation
    IRConstraintInstantiation* getConstraintInstantiation(
        IRConstraintTemplate* constraintTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Fully instantiate a constraint template
    /// @param constraintInstantiation The constraint template to update
    /// @return The updated constraint instantiation
    IRConstraintInstantiation* updateConstraintInstantiation(
        IRConstraintInstantiation* constraintInstantiation
    );

private:
    IRNode* visit(IRFunctionHead* node) override;
    IRNode* visit(IRStructInstantiation* node) override;
    IRNode* visit(IRConstraintInstantiation* node) override;

    IRNode* visit(IRGenericType* node) override;
    IRNode* visit(IRVoidType* node) override { return node; }
    IRNode* visit(IRBoolType* node) override { return node; }
    IRNode* visit(IRIntegerType* node) override { return node; }
    IRNode* visit(IRCharType* node) override { return node; }
    IRNode* visit(IRStringType* node) override { return node; }
    IRNode* visit(IRPointerType* node) override;
    IRNode* visit(IRArrayType* node) override;
};