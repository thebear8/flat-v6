#pragma once
#include "../../ir/ir.hpp"

class GraphContext;
class Environment;

class Instantiator : IRVisitor<IRNode*>
{
private:
    GraphContext& m_envCtx;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    Instantiator(GraphContext& envCtx) : m_envCtx(envCtx) {}

    /// @brief Get the instantiation of the given struct with the type args. If
    /// this instantiation does not yet exist, create it and add it to the
    /// parent module of the struct template.
    /// @param structTemplate The struct template to get an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The struct instantiation
    IRStructInstantiation* getStructInstantiation(
        IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
    );

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

    /// @brief Get the instantiation of the given function with the given type
    /// args. If this instantiation does not yet exist, create it and add it to
    /// the parent module of the function.
    /// @param functionTemplate The function  to get an instantiation
    /// of
    /// @param typeArgs The type args of the instantiation
    /// @return The function instantiation
    IRFunction* getFunctionInstantiation(
        IRFunction* function, std::vector<IRType*> const& typeArgs
    );

    /// @brief Instantiate a type with the generic param values in the given
    /// environment
    /// @param type The type to instantiate
    /// @param env The environment to get generic param values from
    /// @param irCtx The IR context to use for creating type instantiations
    /// @return The instantiated type
    IRType* instantiateType(
        IRType* type, Environment* env, GraphContext* irCtx
    );

private:
    IRNode* visit(IRConstraintFunction* node) override;
    IRNode* visit(IRStructInstantiation* node) override;
    IRNode* visit(IRGenericType* node) override;
    IRNode* visit(IRVoidType* node) override { return node; }
    IRNode* visit(IRBoolType* node) override { return node; }
    IRNode* visit(IRIntegerType* node) override { return node; }
    IRNode* visit(IRCharType* node) override { return node; }
    IRNode* visit(IRStringType* node) override { return node; }
    IRNode* visit(IRPointerType* node) override;
    IRNode* visit(IRArrayType* node) override;
};