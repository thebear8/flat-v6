#include "instantiator.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>

#include "../../util/zip_view.hpp"

IRStructInstantiation* Instantiator::makeStructInstantiation(
    IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
)
{
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

    auto instantiation = m_irCtx->make(IRStructInstantiation(
        structTemplate->name,
        typeArgs,
        std::unordered_map(fields.begin(), fields.end())
    ));

    instantiation->setLocation(structTemplate->getLocation(SourceRef()));
    return instantiation;
}

IRFunctionInstantiation* Instantiator::makeFunctionInstantiation(
    IRFunctionTemplate* functionTemplate, std::vector<IRType*> const& typeArgs
)
{
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

    auto instantiation = m_irCtx->make(IRFunctionInstantiation(
        functionTemplate->name,
        typeArgs,
        std::vector(params.begin(), params.end()),
        result,
        std::vector(requirements.begin(), requirements.end()),
        body
    ));

    instantiation->setLocation(functionTemplate->getLocation(SourceRef()));
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

IRNode* Instantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return nullptr;
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