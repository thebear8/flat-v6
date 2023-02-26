#include "generic_lowering_pass.hpp"

#include <cassert>

void GenericLoweringPass::process(IRModule* node)
{
    dispatch(node);
}

IRNode* GenericLoweringPass::visit(IRIntegerExpression* node)
{
    return node;
}

IRNode* GenericLoweringPass::visit(IRBoolExpression* node)
{
    return node;
}

IRNode* GenericLoweringPass::visit(IRCharExpression* node)
{
    return node;
}

IRNode* GenericLoweringPass::visit(IRStringExpression* node)
{
    return node;
}

IRNode* GenericLoweringPass::visit(IRIdentifierExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRStructExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRUnaryExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRBinaryExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRCallExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRIndexExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRFieldExpression* node)
{
}

IRNode* GenericLoweringPass::visit(IRBlockStatement* node)
{
}

IRNode* GenericLoweringPass::visit(IRExpressionStatement* node)
{
}

IRNode* GenericLoweringPass::visit(IRVariableStatement* node)
{
}

IRNode* GenericLoweringPass::visit(IRReturnStatement* node)
{
}

IRNode* GenericLoweringPass::visit(IRWhileStatement* node)
{
}

IRNode* GenericLoweringPass::visit(IRIfStatement* node)
{
}

IRNode* GenericLoweringPass::visit(IRFunction* node)
{
}

IRNode* GenericLoweringPass::visit(IRModule* node)
{
}

IRType* GenericLoweringPass::inferTypeArg(
    IRGenericType* typeParam,
    IRType* genericType,
    IRType* actualType,
    SourceRef const& errorLocation
)
{
    if (genericType == typeParam)
    {
        return actualType;
    }
    else if (genericType->isStructType())
    {
        if (!actualType->isStructType())
        {
            return m_logger.error(
                errorLocation,
                "actualType has to be the same kind of type as genericType",
                nullptr
            );
        }

        auto genericStructType = (IRStructType*)genericType;
        auto actualStructType = (IRStructType*)actualType;

        if (genericStructType->fields.size() != actualStructType->fields.size())
        {
            return m_logger.error(
                errorLocation,
                "genericStructType and actualStructType have to have the same number of fields",
                nullptr
            );
        }

        IRType* typeArg = nullptr;
        for (auto const& [name, type] : genericStructType->fields)
        {
            if (!actualStructType->fields.contains(name))
            {
                return m_logger.error(
                    errorLocation,
                    "actualStructType has to have the same fields as genericStructType",
                    nullptr
                );
            }

            auto inferredTypeArg = inferTypeArg(
                typeParam,
                type,
                actualStructType->fields.at(name),
                errorLocation
            );

            if (inferredTypeArg != nullptr && typeArg != nullptr
                && inferredTypeArg != typeArg)
            {
                return m_logger.error(
                    errorLocation,
                    "Inferred type arg has to be consistent, was previously "
                        + typeArg->toString() + ", is now "
                        + inferredTypeArg->toString(),
                    nullptr
                );
            }

            typeArg = inferredTypeArg;
        }

        return typeArg;
    }
    else if (genericType->isPointerType())
    {
        if (!actualType->isPointerType())
        {
            return m_logger.error(
                errorLocation,
                "actualType has to be the same kind of type as genericType",
                nullptr
            );
        }

        auto genericPointerType = (IRPointerType*)genericType;
        auto actualPointerType = (IRPointerType*)actualType;

        return inferTypeArg(
            typeParam,
            genericPointerType->base,
            actualPointerType->base,
            errorLocation
        );
    }
    else if (genericType->isArrayType())
    {
        if (!actualType->isArrayType())
        {
            return m_logger.error(
                errorLocation,
                "actualType has to be the same kind of type as genericType",
                nullptr
            );
        }

        auto genericArrayType = (IRArrayType*)genericType;
        auto actualArrayType = (IRArrayType*)actualType;

        return inferTypeArg(
            typeParam,
            genericArrayType->base,
            actualArrayType->base,
            errorLocation
        );
    }

    return nullptr;
}
