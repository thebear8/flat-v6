#include "lowering_pass.hpp"

void* OperatorLoweringPass::process(IRModule* mod)
{
    dispatch(mod);
}

IRNode* OperatorLoweringPass::visit(IRIntegerExpression* node)
{
    return node;
}

IRNode* OperatorLoweringPass::visit(IRBoolExpression* node)
{
    return node;
}

IRNode* OperatorLoweringPass::visit(IRCharExpression* node)
{
    return node;
}

IRNode* OperatorLoweringPass::visit(IRStringExpression* node)
{
    return node;
}

IRNode* OperatorLoweringPass::visit(IRIdentifierExpression* node)
{
    return node;
}

IRNode* OperatorLoweringPass::visit(IRStructExpression* node)
{
    return node;
}

IRNode* OperatorLoweringPass::visit(IRUnaryExpression* node)
{
    node->expression = (IRExpression*)dispatch(node->expression);

    auto value = node->expression->getMD<IRType*>().value();
    if (unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryArithmetic
        && value->isIntegerType())
    {
        return node;
    }
    else if (
        unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryBitwise
        && value->isIntegerType())
    {
        return node;
    }
    else if (
        unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryLogic
        && value->isBoolType())
    {
        return node;
    }
    else
    {
        auto args = std::vector({ node->expression });
        auto identifier = m_irCtx->make(
            IRIdentifierExpression(unaryOperators.at(node->operation).name)
        );
        identifier->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
        auto call = m_irCtx->make(IRCallExpression(identifier, args));
        call->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
        call->setMD(node->getMD<IRType*>().value());
        call->target = node->getMD<IRFunction*>().value();
        return call;
    }
}

IRNode* OperatorLoweringPass::visit(IRBinaryExpression* node)
{
    node->left = (IRExpression*)dispatch(node->left);
    node->right = (IRExpression*)dispatch(node->right);

    auto left = node->left->getMD<IRType*>().value();
    auto right = node->right->getMD<IRType*>().value();

    if (binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryArithmetic
        && (left->isIntegerType() && right->isIntegerType()))
    {
        return node;
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryBitwise
        && (left->isIntegerType() && right->isIntegerType())
        && (left->getBitSize() == right->getBitSize()))
    {
        return node;
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryComparison
        && (left->isIntegerType() && right->isIntegerType()))
    {
        return node;
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryLogic
        && (left->isBoolType() && right->isBoolType()))
    {
        return node;
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryEquality
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        return node;
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryAssign
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        return node;
    }
    else
    {
        if (node->operation == BinaryOperator::Assign)
        {
            auto args = std::vector({ node->left, node->right });
            auto identifier = m_irCtx->make(IRIdentifierExpression(
                binaryOperators.at(BinaryOperator::Assign).name
            ));
            identifier->setMD(node->getMD<SourceRef>().value_or(SourceRef()));

            auto assignCall = m_irCtx->make(IRCallExpression(identifier, args));
            assignCall->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
            assignCall->setMD(node->getMD<IRType*>().value());
            assignCall->target = node->target;

            auto assign = m_irCtx->make(IRBinaryExpression(
                BinaryOperator::Assign, node->left, assignCall
            ));
            assign->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
            assign->setMD(node->getMD<IRType*>().value());
            return assign;
        }
        else
        {
            auto args = std::vector({ node->left, node->right });
            auto identifier = m_irCtx->make(
                IRIdentifierExpression(binaryOperators.at(node->operation).name)
            );
            identifier->setMD(node->getMD<SourceRef>().value_or(SourceRef()));

            auto call = m_irCtx->make(IRCallExpression(identifier, args));
            call->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
            call->setMD(node->getMD<IRType*>().value());
            call->target = node->target;
            return call;
        }
    }
}

IRNode* OperatorLoweringPass::visit(IRCallExpression* node)
{
    node->expression = (IRExpression*)dispatch(node->expression);
    for (auto& arg : node->args)
        arg = (IRExpression*)dispatch(arg);

    if (dynamic_cast<IRIdentifierExpression*>(node->expression))
        return node;

    auto args = node->args;
    args.insert(args.begin(), node->expression);

    auto identifier = m_irCtx->make(IRIdentifierExpression("__call__"));
    identifier->setMD(node->getMD<SourceRef>().value_or(SourceRef()));

    auto call = m_irCtx->make(IRCallExpression(identifier, args));
    call->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
    call->setMD(node->getMD<IRType*>().value());
    call->target = node->target;
    return call;
}

IRNode* OperatorLoweringPass::visit(IRIndexExpression* node)
{
    node->expression = (IRExpression*)dispatch(node->expression);
    for (auto& arg : node->args)
        arg = (IRExpression*)dispatch(arg);

    auto value = node->expression->getMD<IRType*>().value();
    if ((value->isArrayType() || value->isStringType())
        && node->args.size() == 1
        && node->args.front()->getMD<IRType*>().value()->isIntegerType())
        return node;

    auto args = node->args;
    args.insert(args.begin(), node->expression);

    auto identifier = m_irCtx->make(IRIdentifierExpression("__index__"));
    identifier->setMD(node->getMD<SourceRef>().value_or(SourceRef()));

    auto call = m_irCtx->make(IRCallExpression(identifier, args));
    call->setMD(node->getMD<SourceRef>().value_or(SourceRef()));
    call->setMD(node->getMD<IRType*>().value());
    call->target = node->target;
    return call;
}

IRNode* OperatorLoweringPass::visit(IRFieldExpression* node)
{
    node->expression = (IRExpression*)dispatch(node->expression);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRBlockStatement* node)
{
    for (auto& statement : node->statements)
        statement = (IRStatement*)dispatch(statement);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRExpressionStatement* node)
{
    node->expression = (IRExpression*)dispatch(node->expression);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRVariableStatement* node)
{
    for (auto& [name, value] : node->items)
        value = (IRExpression*)dispatch(value);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRReturnStatement* node)
{
    node->expression = (IRExpression*)dispatch(node->expression);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRWhileStatement* node)
{
    node->condition = (IRExpression*)dispatch(node->condition);
    node->body = (IRStatement*)dispatch(node->body);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRIfStatement* node)
{
    node->condition = (IRExpression*)dispatch(node->condition);
    node->ifBody = (IRStatement*)dispatch(node->ifBody);
    node->elseBody =
        (node->elseBody ? (IRStatement*)dispatch(node->elseBody) : nullptr);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRFunction* node)
{
    node->body = ((node->body) ? (IRStatement*)dispatch(node->body) : nullptr);
    return node;
}

IRNode* OperatorLoweringPass::visit(IRModule* node)
{
    for (auto& function : node->functions)
        function = (IRFunction*)dispatch(function);
    return node;
}