#pragma once
#include "../../ir/ir.hpp"

class Environment;
class GraphContext;

class Instantiator : IRVisitor<IRNode*>
{
private:
    Environment* m_env;
    GraphContext* m_irCtx;

public:
    Instantiator() : m_env(nullptr), m_irCtx(nullptr) {}

    /// @brief Create an empty struct instantiation with only name and typeArgs
    /// set. Add this instantiation to the parent module of the template
    /// @param structTemplate The struct template to create an instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The created struct instantiation
    IRStructInstantiation* makeStructInstantiation(
        IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
    );

    /// @brief Create an empty function instantiation with only name, typeArgs,
    /// params and result set. Add this instantiation to the parent module of
    /// the template
    /// @param functionTemplate The function template to create an instantiation
    /// of
    /// @param typeArgs The type args of the instantiation
    /// @return The created function instantiation
    IRFunctionInstantiation* makeFunctionInstantiation(
        IRFunctionTemplate* functionTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Create an empty constraint instantiation with only name and
    /// typeArgs set. Add this instantiation to the parent module of the
    /// template
    /// @param constraintTemplate The constraint template to create an
    /// instantiation of
    /// @param typeArgs The type args of the instantiation
    /// @return The created constraint instantiation
    IRConstraintInstantiation* makeConstraintInstantiation(
        IRConstraintTemplate* constraintTemplate,
        std::vector<IRType*> const& typeArgs
    );

    /// @brief Fully instantiate a struct template
    /// @param structInstantiation The struct instantiation to update
    /// @return The updated struct instantiation
    IRStructInstantiation* updateStructInstantiation(
        IRStructInstantiation* structInstantiation
    );

    /// @brief Fully instantiate a function template
    /// @param functionInstantiation The function instantiation to update
    /// @return The updated function instantiation
    IRFunctionInstantiation* updateFunctionInstantiation(
        IRFunctionInstantiation* functionInstantiation
    );

    /// @brief Fully instantiate a constraint template
    /// @param constraintInstantiation The constraint template to update
    /// @return The updated constraint instantiation
    IRConstraintInstantiation* updateConstraintInstantiation(
        IRConstraintInstantiation* constraintInstantiation
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

    IRNode* visit(IRConstraintCondition* node) override;
    IRNode* visit(IRConstraintInstantiation* node) override;

    IRNode* visit(IRGenericType* node) override;
    IRNode* visit(IRVoidType* node) override { return node; }
    IRNode* visit(IRBoolType* node) override { return node; }
    IRNode* visit(IRIntegerType* node) override { return node; }
    IRNode* visit(IRCharType* node) override { return node; }
    IRNode* visit(IRStringType* node) override { return node; }
    IRNode* visit(IRPointerType* node) override;
    IRNode* visit(IRArrayType* node) override;
    IRNode* visit(IRStructInstantiation* node) override;
};