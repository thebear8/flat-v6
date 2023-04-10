#include "constraint_instantiator.hpp"

#include "../../environment.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "struct_instantiator.hpp"

IRConstraintInstantiation* ConstraintInstantiator::getConstraintInstantiation(
    IRConstraintTemplate* constraintTemplate,
    std::vector<IRType*> const& typeArgs
)
{
    FLC_ASSERT(
        typeArgs.size() == constraintTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    auto instantiation =
        constraintTemplate->getParent()->getEnv()->getConstraintInstantiation(
            constraintTemplate, typeArgs
        );

    if (instantiation)
        return instantiation;

    instantiation = constraintTemplate->getParent()->getIrCtx()->make(
        IRConstraintInstantiation(constraintTemplate->name, typeArgs, {}, {})
    );

    instantiation->setInstantiatedFrom(constraintTemplate);
    instantiation->setLocation(constraintTemplate->getLocation(SourceRef()));

    constraintTemplate->getParent()->getEnv()->addConstraintInstantiation(
        constraintTemplate, instantiation
    );

    return instantiation;
}

IRConstraintInstantiation*
ConstraintInstantiator::updateConstraintInstantiation(
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
            return (IRConstraintInstantiation*)dispatch(r);
        });

    auto conditions =
        constraintTemplate->conditions | std::views::transform([&](auto c) {
            return (IRFunctionHead*)dispatch(c);
        });

    constraintInstantiation->requirements =
        std::set(requirements.begin(), requirements.end());

    constraintInstantiation->conditions =
        std::vector(conditions.begin(), conditions.end());

    m_env = nullptr;
    m_irCtx = nullptr;
    return constraintInstantiation;
}

IRNode* ConstraintInstantiator::visit(IRFunctionHead* node)
{
    auto params = node->params | std::views::transform([&](auto const& p) {
                      return std::pair(p.first, (IRType*)dispatch(p.second));
                  })
        | range_utils::to_vector;
    auto result = (IRType*)dispatch(node->result);

    return m_irCtx->make(IRFunctionHead(node->name, params, result));
}

IRNode* ConstraintInstantiator::visit(IRConstraintInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    })
        | range_utils::to_vector;

    return getConstraintInstantiation(node->getInstantiatedFrom(), typeArgs);
}

IRNode* ConstraintInstantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return m_structInstantiator.getStructInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* ConstraintInstantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return node;
}

IRNode* ConstraintInstantiator::visit(IRPointerType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRPointerType(base));
}

IRNode* ConstraintInstantiator::visit(IRArrayType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRArrayType(base));
}