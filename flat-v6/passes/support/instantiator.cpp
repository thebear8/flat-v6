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
    m_irCtx = structTemplate->getParent()->getIrCtx();
    m_env = &Environment(
        structTemplate->name, structTemplate->getParent()->getEnv()
    );

    assert(
        typeArgs.size() == structTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    auto zippedArgs = zip_view(
        std::views::all(structTemplate->typeParams), std::views::all(typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto fields = structTemplate->fields | std::views::transform([&](auto f) {
                      return std::pair(f.first, (IRType*)dispatch(f.second));
                  });

    m_env = nullptr;
    m_irCtx = nullptr;

    auto instantiation = m_irCtx->make(IRStructInstantiation(
        structTemplate->name,
        typeArgs,
        std::unordered_map(fields.begin(), fields.end())
    ));

    instantiation->setInstantiatedFrom(structTemplate);
    instantiation->setLocation(structTemplate->getLocation(SourceRef()));
    return instantiation;
}

IRStructInstantiation* Instantiator::fixupStructInstantiationFields(
    IRStructInstantiation* structInstantiation
)
{
    auto structTemplate = structInstantiation->getInstantiatedFrom();

    m_irCtx = structTemplate->getParent()->getIrCtx();
    m_env = &Environment(
        structInstantiation->name, structTemplate->getParent()->getEnv()
    );

    auto fields = structTemplate->fields | std::views::transform([&](auto f) {
                      return std::pair(f.first, (IRType*)dispatch(f.second));
                  });

    structInstantiation->fields =
        std::unordered_map(fields.begin(), fields.end());

    m_env = nullptr;
    m_irCtx = nullptr;
}

IRFunctionInstantiation* Instantiator::makeFunctionInstantiation(
    IRFunctionTemplate* functionTemplate, std::vector<IRType*> const& typeArgs
)
{
    m_irCtx = functionTemplate->getParent()->getIrCtx();
    m_env = &Environment(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    );

    assert(
        typeArgs.size() == functionTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    auto zippedArgs = zip_view(
        std::views::all(functionTemplate->typeParams), std::views::all(typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto result = (IRType*)dispatch(functionTemplate->result);
    auto body = (IRStatement*)dispatch(functionTemplate->body);

    auto params =
        functionTemplate->params | std::views::transform([&](auto const& p) {
            return std::pair(p.first, (IRType*)dispatch(p.second));
        });

    auto requirements =
        functionTemplate->requirements
        | std::views::transform([&](auto const& r) {
              auto args = p.second | std::views::transform([&](auto a) {
                              return (IRType*)dispatch(a);
                          });
              return std::pair(p.first, std::vector(args.begin(), args.end()));
          });

    m_env = nullptr;
    m_irCtx = nullptr;

    auto instantiation = m_irCtx->make(IRFunctionInstantiation(
        functionTemplate->name,
        typeArgs,
        std::vector(params.begin(), params.end()),
        result,
        std::vector(requirements.begin(), requirements.end()),
        body
    ));

    instantiation->setInstantiatedFrom(functionTemplate);
    instantiation->setLocation(functionTemplate->getLocation(SourceRef()));
    return instantiation;
}

IRConstraintInstantiation* Instantiator::makeConstraintInstantiation(
    IRConstraintTemplate* constraintTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    m_irCtx = constraintTemplate->getParent()->getIrCtx();
    m_env = &Environment(
        constraintTemplate->name, constraintTemplate->getParent()->getEnv()
    );

    assert(
        typeArgs.size() == constraintTemplate->typeParams.size()
        && "Number of type args has to match number of type params"
    );

    auto zippedArgs = zip_view(
        std::views::all(constraintTemplate->typeParams),
        std::views::all(typeArgs)
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

    m_env = nullptr;
    m_irCtx = nullptr;

    auto instantiation = m_irCtx->make(IRConstraintInstantiation(
        constraintTemplate->name,
        typeArgs,
        std::set(requirements.begin(), requirements.end()),
        std::vector(conditions.begin(), conditions.end())
    ));

    instantiation->setInstantiatedFrom(constraintTemplate);
    instantiation->setLocation(constraintTemplate->getLocation(SourceRef()));
    return instantiation;
}

IRNode* Instantiator::visit(IRIdentifierExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    });

    return m_irCtx->make(IRIdentifierExpression(node->value, typeArgs))
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

    return m_irCtx->make(IRUnaryExpression(node->operation, expression))
        ->setType(type)
        ->setLocation(location);
}

IRNode* Instantiator::visit(IRBinaryExpression* node)
{
    auto type = (IRType*)dispatch(node->getType());
    auto location = node->getLocation(SourceRef());
    auto left = (IRExpression*)dispatch(node->left);
    auto right = (IRExpression*)dispatch(node->right);

    return m_irCtx->make(IRBinaryExpression(node->operation, left, right))
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

    return m_irCtx
        ->make(
            IRCallExpression(expression, std::vector(args.begin(), args.end()))
        )
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

    return m_irCtx
        ->make(
            IRIndexExpression(expression, std::vector(args.begin(), args.end()))
        )
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

    auto requirements = node->requirements | std::views::transform([&](auto r) {
                            return (IRConstraintInstantiation*)dispatch(r);
                        });

    auto conditions = node->conditions | std::views::transform([&](auto c) {
                          return (IRConstraintCondition*)dispatch(c);
                      });

    auto instantiation = m_irCtx->make(IRConstraintInstantiation(
        node->name,
        typeArgs,
        std::set(requirements.begin(), requirements.end()),
        std::vector(conditions.begin(), conditions.end())
    ));

    instantiation->setInstantiatedFrom(node->getInstantiatedFrom());
    instantiation->setLocation(node->getLocation(SourceRef()));
    return instantiation;
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

IRNode* Instantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    });
    auto fields = node->fields | std::views::transform([&](auto f) {
                      return std::pair(f.first, (IRType*)dispatch(f.second));
                  });

    return m_irCtx
        ->make(IRStructInstantiation(
            node->name,
            std::vector(typeArgs.begin(), typeArgs.end()),
            std::unordered_map(fields.begin(), fields.end())
        ))
        ->setLocation(node->getLocation(SourceRef()));
}