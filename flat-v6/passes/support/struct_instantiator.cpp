#include "struct_instantiator.hpp"

#include "../../environment.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"

IRStructInstantiation* StructInstantiator::getStructInstantiation(
    IRStructTemplate* structTemplate, std::vector<IRType*> const& typeArgs
)
{
    FLC_ASSERT(
        typeArgs.size() == structTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    auto instantiation =
        structTemplate->getParent()->getEnv()->getStructInstantiation(
            structTemplate, typeArgs
        );

    if (instantiation)
        return instantiation;

    instantiation = structTemplate->getParent()->getIrCtx()->make(
        IRStructInstantiation(structTemplate->name, typeArgs, {})
    );

    instantiation->setInstantiatedFrom(structTemplate);
    instantiation->setLocation(structTemplate->getLocation(SourceRef()));

    structTemplate->getParent()->getEnv()->addStructInstantiation(
        structTemplate, instantiation
    );

    return instantiation;
}

IRStructInstantiation* StructInstantiator::updateStructInstantiation(
    IRStructInstantiation* structInstantiation
)
{
    auto structTemplate = structInstantiation->getInstantiatedFrom();

    FLC_ASSERT(
        structInstantiation->typeArgs.size()
            == structTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    m_env = m_envCtx.make(
        Environment(structTemplate->name, structTemplate->getParent()->getEnv())
    );
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
        tsl::ordered_map<std::string, IRType*>(fields.begin(), fields.end());

    m_env = nullptr;
    m_irCtx = nullptr;
    return structInstantiation;
}

IRNode* StructInstantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return getStructInstantiation(node->getInstantiatedFrom(), typeArgs);
}

IRNode* StructInstantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return node;
}

IRNode* StructInstantiator::visit(IRPointerType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRPointerType(base));
}

IRNode* StructInstantiator::visit(IRArrayType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRArrayType(base));
}