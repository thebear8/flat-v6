#include "constraint_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "../support/instantiator.hpp"

void ConstraintInstantiationUpdatePass::process(IRModule* node)
{
    for (auto [constraintTemplate, constraintInstantiation] :
         node->getEnv()->getConstraintInstantiationMap())
    {
        update(constraintInstantiation);
    }
}

IRConstraintInstantiation* ConstraintInstantiationUpdatePass::update(
    IRConstraintInstantiation* constraintInstantiation
)
{
    auto constraintTemplate = constraintInstantiation->getInstantiatedFrom();

    m_env = m_envCtx.make(Environment(
        constraintTemplate->name, constraintTemplate->getParent()->getEnv()
    ));
    m_irCtx = constraintTemplate->getParent()->getIrCtx();

    FLC_ASSERT(
        constraintInstantiation->typeArgs.size()
            == constraintTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    auto zippedArgs = zip_view(
        std::views::all(constraintTemplate->typeParams),
        std::views::all(constraintInstantiation->typeArgs)
    );

    for (auto [typeParam, typeArg] : zippedArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    auto requirements =
        constraintTemplate->requirements | std::views::transform([&](auto r) {
            auto typeArgs =
                r->typeArgs | std::views::transform([&](auto arg) {
                    return m_instantiator.instantiateType(arg, m_env, m_irCtx);
                })
                | range_utils::to_vector;

            return m_instantiator.getConstraintInstantiation(
                r->getInstantiatedFrom(), typeArgs
            );
        });

    auto conditions = constraintTemplate->conditions
        | std::views::transform(
                          [&](auto c) {
        auto params =
            c->params | std::views::transform([&](auto const& param) {
                return std::pair(
                    param.first,
                    m_instantiator.instantiateType(param.second, m_env, m_irCtx)
                );
            })
            | range_utils::to_vector;
        auto result = m_instantiator.instantiateType(c->result, m_env, m_irCtx);

        return m_irCtx->make(IRFunctionHead(c->name, params, result));
          });

    constraintInstantiation->requirements =
        std::set(requirements.begin(), requirements.end());

    constraintInstantiation->conditions =
        std::vector(conditions.begin(), conditions.end());

    m_env = nullptr;
    m_irCtx = nullptr;
    return constraintInstantiation;
}