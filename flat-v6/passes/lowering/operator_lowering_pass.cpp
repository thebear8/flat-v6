#include "operator_lowering_pass.hpp"

#include "../../environment.hpp"
#include "../../util/graph_context.hpp"

void OperatorLoweringPass::process(IRModule* node)
{
    m_module = node;
    m_irCtx = node->getIrCtx();

    for (auto& [name, function] : node->getEnv()->getFunctionMap())
        dispatchRef(function);

    m_irCtx = nullptr;
    m_module = nullptr;
}

void OperatorLoweringPass::visit(IRStructExpression* node)
{
    for (auto& [name, value] : node->fields)
        dispatchRef(value);
}

void OperatorLoweringPass::visit(IRUnaryExpression* node, IRNode*& ref)
{
    dispatchRef(node->expression);
    auto name = unaryOperators.at(node->operation).name;
    auto args = std::vector({ node->expression });
    auto call = m_irCtx->make(IRLoweredCallExpression(name, {}, args));
    call->setLocation(node->getLocation(SourceRef()));
    ref = call;
}

void OperatorLoweringPass::visit(IRBinaryExpression* node, IRNode*& ref)
{
    dispatchRef(node->left);
    dispatchRef(node->right);
    auto name = binaryOperators.at(node->operation).name;
    auto args = std::vector({ node->left, node->right });
    auto call = m_irCtx->make(IRLoweredCallExpression(name, {}, args));
    call->setLocation(node->getLocation(SourceRef()));
    ref = call;
}

void OperatorLoweringPass::visit(IRCallExpression* node, IRNode*& ref)
{
    dispatchRef(node->expression);
    for (auto& arg : node->args)
        dispatchRef(arg);

    if (auto identifier =
            dynamic_cast<IRIdentifierExpression*>(node->expression))
    {
        auto name = identifier->value;
        auto typeArgs = identifier->typeArgs;
        auto args = node->args;
        auto call =
            m_irCtx->make(IRLoweredCallExpression(name, typeArgs, args));
        call->setLocation(node->getLocation(SourceRef()));
        ref = call;
    }
    else
    {
        auto name = "__call__";
        auto args = node->args;
        args.insert(args.begin(), node->expression);
        auto call =
            m_irCtx->make(IRLoweredCallExpression(name, {}, node->args));
        call->setLocation(node->getLocation(SourceRef()));
        ref = call;
    }
}

void OperatorLoweringPass::visit(IRIndexExpression* node, IRNode*& ref)
{
    dispatchRef(node->expression);
    for (auto& arg : node->args)
        dispatchRef(arg);

    auto name = "__index__";
    auto args = node->args;
    args.insert(args.begin(), node->expression);
    auto call = m_irCtx->make(IRLoweredCallExpression(name, {}, args));
    call->setLocation(node->getLocation(SourceRef()));
    ref = call;
}

void OperatorLoweringPass::visit(IRFieldExpression* node)
{
    dispatchRef(node->expression);
}

void OperatorLoweringPass::visit(IRBlockStatement* node)
{
    for (auto& statement : node->statements)
        dispatchRef(statement);
}

void OperatorLoweringPass::visit(IRExpressionStatement* node)
{
    dispatchRef(node->expression);
}

void OperatorLoweringPass::visit(IRVariableStatement* node)
{
    for (auto& [name, value] : node->items)
        dispatchRef(value);
}

void OperatorLoweringPass::visit(IRReturnStatement* node)
{
    if (node->expression)
        dispatchRef(node->expression);
}

void OperatorLoweringPass::visit(IRWhileStatement* node)
{
    dispatchRef(node->condition);
    dispatchRef(node->body);
}

void OperatorLoweringPass::visit(IRIfStatement* node)
{
    dispatchRef(node->condition);
    dispatchRef(node->ifBody);
    if (node->elseBody)
        dispatchRef(node->elseBody);
}

void OperatorLoweringPass::visit(IRNormalFunction* node)
{
    if (!node->getExtern(false))
        dispatchRef(node->body);
}