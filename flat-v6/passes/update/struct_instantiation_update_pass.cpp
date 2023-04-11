#include "struct_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "../support/instantiator.hpp"

void StructInstantiationUpdatePass::process(IRModule* node)
{
    for (auto [structTemplate, structInstantiation] :
         node->getEnv()->getStructInstantiationMap())
    {
        update(structInstantiation);
    }
}

IRStructInstantiation* StructInstantiationUpdatePass::update(
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

    auto fields =
        structTemplate->fields | std::views::transform([&](auto f) {
            return std::pair(
                f.first,
                m_instantiator.instantiateType(f.second, m_env, m_irCtx)
            );
        });

    structInstantiation->fields =
        tsl::ordered_map<std::string, IRType*>(fields.begin(), fields.end());

    m_env = nullptr;
    m_irCtx = nullptr;
    return structInstantiation;
}