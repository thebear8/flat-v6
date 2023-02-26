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
    IRGenericType* typeParam, IRType* genericType, IRType* actualType
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
            assert(
                0 && "actualType has to be the same kind of type as genericType"
            );
        }

        auto genericStructType = (IRStructType*)genericType;
        auto actualStructType = (IRStructType*)actualType;

        if (genericStructType->fields.size() != actualStructType->fields.size())
        {
            assert(
                0
                && "genericStructType and actualStructType have to have the same number of fields"
            );
        }

        IRType* typeArg = nullptr;
        for (size_t i = 0; i < genericStructType->fields.size(); i++)
        {
            auto inferredTypeArg = inferTypeArg(
                typeParam,
                genericStructType->fields.at(i).second,
                actualStructType->fields.at(i).second
            );

            if (inferredTypeArg != nullptr && typeArg != nullptr
                && inferredTypeArg != typeArg)
            {
                assert(0 && "Inferred type args have to the same");
            }
        }

        return typeArg;
    }
    else if (genericType->isPointerType())
    {
        if (!actualType->isPointerType())
        {
            assert(
                0 && "actualType has to be the same kind of type as genericType"
            );
        }

        auto genericPointerType = (IRPointerType*)genericType;
        auto actualPointerType = (IRPointerType*)actualType;

        return inferTypeArg(
            typeParam, genericPointerType->base, actualPointerType->base
        );
    }
    else if (genericType->isArrayType())
    {
        if (!actualType->isArrayType())
        {
            assert(
                0 && "actualType has to be the same kind of type as genericType"
            );
        }

        auto genericArrayType = (IRArrayType*)genericType;
        auto actualArrayType = (IRArrayType*)actualType;

        return inferTypeArg(
            typeParam, genericArrayType->base, actualArrayType->base
        );
    }

    return nullptr;
}
