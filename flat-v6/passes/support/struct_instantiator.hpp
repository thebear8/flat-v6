#pragma once
#include "../../ir/ir.hpp"

class Environment;
class GraphContext;

class StructInstantiator : IRVisitor<IRNode*>
{
private:
    GraphContext& m_envCtx;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    StructInstantiator(GraphContext& envCtx) : m_envCtx(envCtx) {}

    /// @brief Get the instantiation of the given struct with the type args. If
    /// this instantiation does not yet exist, create it and add it to the
    /// parent module of the struct template.
    /// @param structTemplate The struct template to get an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The struct instantiation
    IRStructInstantiation* getStructInstantiation(
        IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
    );

    /// @brief Fully instantiate a struct template
    /// @param structInstantiation The struct instantiation to update
    /// @return The updated struct instantiation
    IRStructInstantiation* updateStructInstantiation(
        IRStructInstantiation* structInstantiation
    );

private:
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