#include "instantiator.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <vector>

#include "../../compiler.hpp"
#include "../../ir/ir.hpp"
#include "../../util/error_logger.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/zip_view.hpp"

IRStructInstantiation* Instantiator::makeStructInstantiation(
    IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
)
{
    assert(
        typeArgs.size() == structTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    auto instantiation = structTemplate->getParent()->getIrCtx()->make(
        IRStructInstantiation(structTemplate->name, typeArgs, {})
    );

    instantiation->setInstantiatedFrom(structTemplate);
    instantiation->setLocation(structTemplate->getLocation(SourceRef()));

    structTemplate->getParent()->getEnv()->addStructInstantiation(
        structTemplate, instantiation
    );

    return instantiation;
}

IRFunctionInstantiation* Instantiator::makeFunctionInstantiation(
    IRFunctionTemplate* functionTemplate, std::vector<IRType*> const& typeArgs
)
{
    assert(
        typeArgs.size() == functionTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    Environment env(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    );
    m_env = &env;
    m_irCtx = functionTemplate->getParent()->getIrCtx();

    auto zippedTypeArgs = zip_view(
        std::views::all(functionTemplate->typeParams), std::views::all(typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedTypeArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto params =
        functionTemplate->params | std::views::transform([&](auto const& p) {
            return std::pair(p.first, (IRType*)dispatch(p.second));
        });

    auto result = (IRType*)dispatch(functionTemplate->result);

    m_env = nullptr;
    m_irCtx = nullptr;

    auto instantiation = m_irCtx->make(IRFunctionInstantiation(
        functionTemplate->name,
        typeArgs,
        std::vector(params.begin(), params.end()),
        result,
        {},
        nullptr
    ));

    instantiation->setInstantiatedFrom(functionTemplate);
    instantiation->setLocation(functionTemplate->getLocation(SourceRef()));

    functionTemplate->getParent()->getEnv()->addFunctionInstantiation(
        functionTemplate, instantiation
    );

    return instantiation;
}

IRConstraintInstantiation* Instantiator::makeConstraintInstantiation(
    IRConstraintTemplate* constraintTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    assert(
        typeArgs.size() == constraintTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    auto instantiation = constraintTemplate->getParent()->getIrCtx()->make(
        IRConstraintInstantiation(constraintTemplate->name, typeArgs, {}, {})
    );

    instantiation->setInstantiatedFrom(constraintTemplate);
    instantiation->setLocation(constraintTemplate->getLocation(SourceRef()));

    constraintTemplate->getParent()->getEnv()->addConstraintInstantiation(
        constraintTemplate, instantiation
    );

    return instantiation;
}

IRStructInstantiation* Instantiator::updateStructInstantiation(
    IRStructInstantiation* structInstantiation
)
{
    auto structTemplate = structInstantiation->getInstantiatedFrom();

    assert(
        structInstantiation->typeArgs.size()
            == structTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    Environment env(
        structTemplate->name, structTemplate->getParent()->getEnv()
    );
    m_env = &env;
    m_irCtx = structTemplate->getParent()->getIrCtx();

    auto zippedTypeArgs = zip_view(
        std::views::all(structTemplate->typeParams),
        std::views::all(structInstantiation->typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedTypeArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto fields = structTemplate->fields | std::views::transform([&](auto f) {
                      return std::pair(f.first, (IRType*)dispatch(f.second));
                  });

    structInstantiation->fields =
        std::unordered_map(fields.begin(), fields.end());

    m_env = nullptr;
    m_irCtx = nullptr;
}

IRFunctionInstantiation* Instantiator::updateFunctionInstantiation(
    IRFunctionInstantiation* functionInstantiation
)
{
    auto functionTemplate = functionInstantiation->getInstantiatedFrom();

    assert(
        functionInstantiation->typeArgs.size()
            == functionTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    Environment env(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    );
    m_env = &env;
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
}

IRConstraintInstantiation* Instantiator::updateConstraintInstantiation(
    IRConstraintInstantiation* constraintInstantiation
)
{
    auto constraintTemplate = constraintInstantiation->getInstantiatedFrom();

    m_irCtx = constraintTemplate->getParent()->getIrCtx();
    m_env = &Environment(
        constraintTemplate->name, constraintTemplate->getParent()->getEnv()
    );

    assert(
        constraintInstantiation->typeArgs.size()
            == constraintTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    auto zippedArgs = zip_view(
        std::views::all(constraintTemplate->typeParams),
        std::views::all(constraintInstantiation->typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto requirements =
        constraintTemplate->requirements | std::views::transform([&](auto r) {
            return (IRConstraintInstantiation*)dispatch(r);
        });

    auto conditions =
        constraintTemplate->conditions | std::views::transform([&](auto c) {
            return (IRConstraintCondition*)dispatch(c);
        });

    constraintInstantiation->requirements =
        std::set(requirements.begin(), requirements.end());

    constraintInstantiation->conditions =
        std::vector(conditions.begin(), conditions.end());

    m_env = nullptr;
    m_irCtx = nullptr;
    return constraintInstantiation;
}

IRNode* Instantiator::visit(IRIdentifierExpression* node)
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

IRNode* Instantiator::visit(IRStructExpression* node)
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

IRNode* Instantiator::visit(IRUnaryExpression* node)
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

IRNode* Instantiator::visit(IRBinaryExpression* node)
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

IRNode* Instantiator::visit(IRCallExpression* node)
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

IRNode* Instantiator::visit(IRIndexExpression* node)
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

IRNode* Instantiator::visit(IRFieldExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRFieldExpression(expression, node->fieldName))
        ->setType(type)
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRBlockStatement* node)
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

IRNode* Instantiator::visit(IRExpressionStatement* node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRExpressionStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRVariableStatement* node)
{
    auto items =
        node->items | std::views::transform([&](auto i) {
            return std::pair(i.first, (IRExpression*)dispatch(i.second));
        });

    return m_irCtx
        ->make(IRVariableStatement(std::vector(items.begin(), items.end())))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRReturnStatement* node)
{
    auto expression = (IRExpression*)dispatch(node->expression);

    return m_irCtx->make(IRReturnStatement(expression))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRWhileStatement* node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto body = (IRStatement*)dispatch(node->body);

    return m_irCtx->make(IRWhileStatement(condition, body))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRIfStatement* node)
{
    auto condition = (IRExpression*)dispatch(node->condition);
    auto ifBody = (IRStatement*)dispatch(node->ifBody);
    auto elseBody =
        node->elseBody ? (IRStatement*)dispatch(node->elseBody) : nullptr;

    return m_irCtx->make(IRIfStatement(condition, ifBody, elseBody))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRConstraintCondition* node)
{
    auto params =
        node->params | std::views::transform([&](auto p) {
            return std::make_pair(p.first, (IRType*)dispatch(p.second));
        });

    auto result = (IRType*)dispatch(node->result);

    return m_irCtx
        ->make(IRConstraintCondition(
            node->functionName,
            std::vector(params.begin(), params.end()),
            result
        ))
        ->setLocation(node->getLocation(SourceRef()));
}

IRNode* Instantiator::visit(IRConstraintInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    });

    auto constraintTemplate = node->getInstantiatedFrom();
    auto env = constraintTemplate->getParent()->getEnv();

    auto instantiation = env->findConstraintInstantiation(
        constraintTemplate, std::vector(typeArgs.begin(), typeArgs.end())
    );

    if (instantiation)
        return instantiation;

    return makeConstraintInstantiation(
        constraintTemplate, std::vector(typeArgs.begin(), typeArgs.end())
    );
}

IRNode* Instantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    });

    auto structTemplate = node->getInstantiatedFrom();
    auto env = structTemplate->getParent()->getEnv();

    auto instantiation = env->findStructInstantiation(
        structTemplate, std::vector(typeArgs.begin(), typeArgs.end())
    );

    if (instantiation)
        return instantiation;

    return makeStructInstantiation(
        structTemplate, std::vector(typeArgs.begin(), typeArgs.end())
    );
}

IRNode* Instantiator::visit(IRFunctionInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    });

    auto functionTemplate = node->getInstantiatedFrom();
    auto env = functionTemplate->getParent()->getEnv();

    auto instantiation = env->findFunctionInstantiation(
        functionTemplate, std::vector(typeArgs.begin(), typeArgs.end())
    );

    if (instantiation)
        return instantiation;

    return makeFunctionInstantiation(
        functionTemplate, std::vector(typeArgs.begin(), typeArgs.end())
    );
}

IRNode* Instantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return node;
}

IRNode* Instantiator::visit(IRPointerType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRPointerType(base));
}

IRNode* Instantiator::visit(IRArrayType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRArrayType(base));
}