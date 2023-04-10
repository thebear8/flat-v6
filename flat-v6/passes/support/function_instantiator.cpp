#include "function_instantiator.hpp"

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
#include "constraint_instantiator.hpp"
#include "struct_instantiator.hpp"

IRFunctionInstantiation* FunctionInstantiator::getFunctionInstantiation(
    IRFunctionTemplate* functionTemplate, std::vector<IRType*> const& typeArgs
)
{
    FLC_ASSERT(
        typeArgs.size() == functionTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    m_env = m_envCtx.make(Environment(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    ));
    m_irCtx = functionTemplate->getParent()->getIrCtx();

    auto zippedTypeArgs = zip_view(
        std::views::all(functionTemplate->typeParams), std::views::all(typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedTypeArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto params = functionTemplate->params
        | std::views::transform([&](auto const& p) {
                      return std::pair(p.first, (IRType*)dispatch(p.second));
                  })
        | range_utils::to_vector;

    auto result = (IRType*)dispatch(functionTemplate->result);

    auto instantiation = m_irCtx->make(IRFunctionInstantiation(
        functionTemplate->name, typeArgs, params, result, {}, nullptr
    ));

    instantiation->setInstantiatedFrom(functionTemplate);
    instantiation->setLocation(functionTemplate->getLocation(SourceRef()));

    functionTemplate->getParent()->getEnv()->addFunctionInstantiation(
        functionTemplate, instantiation
    );

    m_env = nullptr;
    m_irCtx = nullptr;
    return instantiation;
}

IRFunctionInstantiation* FunctionInstantiator::updateFunctionInstantiation(
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

    auto requirements =
        functionTemplate->requirements | std::views::transform([&](auto r) {
            return (IRConstraintInstantiation*)dispatch(r);
        });

    functionInstantiation->requirements =
        std::set(requirements.begin(), requirements.end());

    functionInstantiation->body =
        (IRStatement*)dispatch(functionTemplate->body);

    m_env = nullptr;
    m_irCtx = nullptr;
    return functionInstantiation;
}

IRNode* FunctionInstantiator::visit(IRIdentifierExpression* node)
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

IRNode* FunctionInstantiator::visit(IRStructExpression* node)
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

IRNode* FunctionInstantiator::visit(IRUnaryExpression* node)
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

IRNode* FunctionInstantiator::visit(IRBinaryExpression* node)
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

IRNode* FunctionInstantiator::visit(IRCallExpression* node)
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

IRNode* FunctionInstantiator::visit(IRIndexExpression* node)
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

IRNode* FunctionInstantiator::visit(IRFieldExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRFieldExpression(expression, node->fieldName))
        ->setType(type)
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiator::visit(IRBlockStatement* node)
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

IRNode* FunctionInstantiator::visit(IRExpressionStatement* node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRExpressionStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiator::visit(IRVariableStatement* node)
{
    auto items =
        node->items | std::views::transform([&](auto i) {
            return std::pair(i.first, (IRExpression*)dispatch(i.second));
        });

    return m_irCtx
        ->make(IRVariableStatement(std::vector(items.begin(), items.end())))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiator::visit(IRReturnStatement* node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRReturnStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiator::visit(IRWhileStatement* node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto body = (IRStatement*)dispatch(node->body);

    return m_irCtx->make(IRWhileStatement(condition, body))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiator::visit(IRIfStatement* node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto ifBody = (IRStatement*)dispatch(node->ifBody);
    auto elseBody =
        node->elseBody ? (IRStatement*)dispatch(node->elseBody) : nullptr;

    return m_irCtx->make(IRIfStatement(condition, ifBody, elseBody))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* FunctionInstantiator::visit(IRFunctionHead* node)
{
    auto args = node->params | std::views::transform([&](auto const& p) {
                    return (IRType*)dispatch(p.second);
                })
        | range_utils::to_vector;
    auto result = (IRType*)dispatch(node->result);

    std::vector<IRType*> typeArgs;
    auto target = m_env->findMatchingFunctionTemplate(
        node->name, {}, args, result, typeArgs
    );
    FLC_ASSERT(target, "Target for constraint condition has to exist.");

    return getFunctionInstantiation(target, typeArgs);
}

IRNode* FunctionInstantiator::visit(IRConstraintInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    })
        | range_utils::to_vector;

    return m_constraintInstantiator.getConstraintInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* FunctionInstantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return m_structInstantiator.getStructInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* FunctionInstantiator::visit(IRFunctionInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return getFunctionInstantiation(node->getInstantiatedFrom(), typeArgs);
}

IRNode* FunctionInstantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return node;
}

IRNode* FunctionInstantiator::visit(IRPointerType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRPointerType(base));
}

IRNode* FunctionInstantiator::visit(IRArrayType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRArrayType(base));
}