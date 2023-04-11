#include "function_body_instantiator.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <vector>

#include "../../compiler.hpp"
#include "../../ir/ir.hpp"
#include "../../util/assert.hpp"
#include "../../util/error_logger.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "call_target_resolver.hpp"
#include "constraint_instantiator.hpp"
#include "function_instantiator.hpp"
#include "struct_instantiator.hpp"

IRFunctionInstantiation* FunctionBodyInstantiator::updateBody(
    IRFunctionInstantiation* functionInstantiation
)
{
    auto functionTemplate = functionInstantiation->getInstantiatedFrom();

    FLC_ASSERT(
        functionInstantiation->typeArgs.size()
            == functionTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    m_env = m_envCtx.make(Environment(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    ));
    m_irCtx = functionTemplate->getParent()->getIrCtx();

    auto zippedTypeArgs = zip_view(
        std::views::all(functionTemplate->typeParams),
        std::views::all(functionInstantiation->typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedTypeArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    functionInstantiation->body =
        (IRStatement*)dispatch(functionTemplate->body);

    m_env = nullptr;
    m_irCtx = nullptr;
    return functionInstantiation;
}

IRNode* FunctionBodyInstantiator::visit(IRIdentifierExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    });

    return m_irCtx
        ->make(IRIdentifierExpression(
            node->value, std::vector(typeArgs.begin(), typeArgs.end())
        ))
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionBodyInstantiator::visit(IRStructExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    });
    auto fields =
        node->fields | std::views::transform([&](auto const& f) {
            return std::pair(f.first, (IRExpression*)dispatch(f.second));
        });

    return m_irCtx
        ->make(IRStructExpression(
            node->structName,
            std::vector(typeArgs.begin(), typeArgs.end()),
            std::unordered_map(fields.begin(), fields.end())
        ))
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionBodyInstantiator::visit(IRUnaryExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto expression = (IRExpression*)dispatch(node->expression);
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx->make(IRUnaryExpression(node->operation, expression))
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionBodyInstantiator::visit(IRBinaryExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto left = (IRExpression*)dispatch(node->left);
    auto right = (IRExpression*)dispatch(node->right);
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx->make(IRBinaryExpression(node->operation, left, right))
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionBodyInstantiator::visit(IRCallExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto expression = (IRExpression*)dispatch(node->expression);
    auto args = node->args | std::views::transform([&](auto a) {
                    return (IRExpression*)dispatch(a);
                });
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx
        ->make(
            IRCallExpression(expression, std::vector(args.begin(), args.end()))
        )
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionBodyInstantiator::visit(IRIndexExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto expression = (IRExpression*)dispatch(node->expression);
    auto args = node->args | std::views::transform([&](auto a) {
                    return (IRExpression*)dispatch(a);
                });
    auto target = (IRFunctionInstantiation*)dispatch(node->getTarget());

    return m_irCtx
        ->make(
            IRIndexExpression(expression, std::vector(args.begin(), args.end()))
        )
        ->setTarget(target)
        ->setType(type)
        ->setLocation(location);
}

IRNode* FunctionBodyInstantiator::visit(IRFieldExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRFieldExpression(expression, node->fieldName))
        ->setType(type)
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRBlockStatement* node)
{
    auto statements = node->statements | std::views::transform([&](auto s) {
                          return (IRStatement*)dispatch(s);
                      });

    return m_irCtx
        ->make(
            IRBlockStatement(std::vector(statements.begin(), statements.end()))
        )
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRExpressionStatement* node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRExpressionStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRVariableStatement* node)
{
    auto items =
        node->items | std::views::transform([&](auto i) {
            return std::pair(i.first, (IRExpression*)dispatch(i.second));
        });

    return m_irCtx
        ->make(IRVariableStatement(std::vector(items.begin(), items.end())))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRReturnStatement* node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRReturnStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRWhileStatement* node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto body = (IRStatement*)dispatch(node->body);

    return m_irCtx->make(IRWhileStatement(condition, body))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRIfStatement* node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto ifBody = (IRStatement*)dispatch(node->ifBody);
    auto elseBody =
        node->elseBody ? (IRStatement*)dispatch(node->elseBody) : nullptr;

    return m_irCtx->make(IRIfStatement(condition, ifBody, elseBody))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionBodyInstantiator::visit(IRFunctionHead* node)
{
    auto args = node->params | std::views::transform([&](auto const& p) {
                    return (IRType*)dispatch(p.second);
                })
        | range_utils::to_vector;
    auto result = (IRType*)dispatch(node->result);

    std::vector<IRType*> typeArgs;
    auto target = m_callTargetResolver.findMatchingFunctionTemplate(
        m_env, node->name, {}, args, result, typeArgs
    );
    FLC_ASSERT(target, "Target for constraint condition has to exist.");

    return m_functionInstantiator.getFunctionInstantiation(target, typeArgs);
}

IRNode* FunctionBodyInstantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return m_structInstantiator.getStructInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* FunctionBodyInstantiator::visit(IRFunctionInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return m_functionInstantiator.getFunctionInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* FunctionBodyInstantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return node;
}

IRNode* FunctionBodyInstantiator::visit(IRPointerType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRPointerType(base));
}

IRNode* FunctionBodyInstantiator::visit(IRArrayType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRArrayType(base));
}