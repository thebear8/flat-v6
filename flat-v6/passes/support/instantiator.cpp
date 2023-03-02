#include "instantiator.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>

#include "../../util/zip_view.hpp"

IRStructInstantiation* Instantiator::makeStructInstantiation(
    IRModule* irModule,
    IRStructTemplate* structTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    m_env = &Environment(structTemplate->name, irModule->getEnv());

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

    return m_irCtx->make(IRStructInstantiation(
        structTemplate->name,
        typeArgs,
        std::unordered_map(fields.begin(), fields.end())
    ));
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

    return m_irCtx->make(IRStructInstantiation(
        node->name,
        std::vector(typeArgs.begin(), typeArgs.end()),
        std::unordered_map(fields.begin(), fields.end())
    ));
}