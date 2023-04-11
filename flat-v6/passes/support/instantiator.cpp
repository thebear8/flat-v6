#include "instantiator.hpp"

#include "../../environment.hpp"
#include "../../util/assert.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"

IRStructInstantiation* Instantiator::getStructInstantiation(
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

IRConstraintInstantiation* Instantiator::getConstraintInstantiation(
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

IRFunctionInstantiation* Instantiator::getFunctionInstantiation(
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
                      return std::pair(p.first, instantiateType(p.second));
                  })
        | range_utils::to_vector;

    auto result = instantiateType(functionTemplate->result);

    auto requirements = functionTemplate->requirements
        | std::views::transform(
                            [&](auto r) {
        auto typeArgs = r->typeArgs | std::views::transform([&](auto arg) {
                            return instantiateType(arg);
                        })
            | range_utils::to_vector;

        return getConstraintInstantiation(r->getInstantiatedFrom(), typeArgs);
          });

    auto instantiation = m_irCtx->make(IRFunctionInstantiation(
        functionTemplate->name,
        typeArgs,
        params,
        result,
        std::set(requirements.begin(), requirements.end()),
        nullptr
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

IRType* Instantiator::instantiateType(IRType* type)
{
    return (IRType*)dispatch(type);
}

IRType* Instantiator::instantiateType(
    IRType* type, Environment* env, GraphContext* irCtx
)
{
    auto prevEnv = m_env;
    auto prevIrCtx = m_irCtx;
    m_env = env;
    m_irCtx = irCtx;

    auto instantiation = instantiateType(type);

    m_env = prevEnv;
    m_irCtx = prevIrCtx;
    return instantiation;
}

IRNode* Instantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return getStructInstantiation(node->getInstantiatedFrom(), typeArgs);
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