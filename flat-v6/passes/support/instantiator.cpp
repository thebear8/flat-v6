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

IRFunction* Instantiator::getFunctionInstantiation(
    IRFunction* function, std::vector<IRType*> const& typeArgs
)
{
    if (function->isConstraintFunction())
    {
        FLC_ASSERT(typeArgs.size() == 0);
        FLC_ASSERT(function->typeParams.size() == 0);
        FLC_ASSERT(function->typeArgs.size() == function->typeParams.size());

        return function;
    }

    FLC_ASSERT(function->isIntrinsicFunction() || function->isNormalFunction());

    FLC_ASSERT(function->parent);
    FLC_ASSERT(!function->blueprint);
    FLC_ASSERT(function->typeArgs.size() == 0);
    FLC_ASSERT(typeArgs.size() == function->typeParams.size());

    if (typeArgs.size() == 0)
        return function;

    auto instantiation = function->parent->getEnv()->getFunctionInstantiation(
        function, typeArgs
    );
    if (instantiation)
        return instantiation;

    m_env = m_envCtx.make(Environment(function->name));
    auto prevIrCtx = m_irCtx;
    m_irCtx = function->parent->getIrCtx();

    if (function->isIntrinsicFunction())
        instantiation = m_irCtx->make(IRIntrinsicFunction());
    else if (function->isNormalFunction())
        instantiation = m_irCtx->make(IRNormalFunction());
    else
        FLC_ASSERT(false);

    instantiation->parent = function->parent;
    instantiation->blueprint = function;
    instantiation->name = function->name;
    instantiation->typeParams = function->typeParams;
    instantiation->typeArgs = typeArgs;

    auto zippedTypeArgs = zip(function->typeParams, typeArgs);
    for (auto [typeParam, typeArg] : zippedTypeArgs)
        m_env->addTypeParamValue(typeParam, typeArg);

    instantiation->params =
        function->params | std::views::transform([&](auto const& p) {
            return std::pair(p.first, (IRType*)dispatch(p.second));
        })
        | range_utils::to_vector;

    instantiation->result = (IRType*)dispatch(function->result);

    instantiation->requirements =
        function->requirements | std::views::transform([&](auto r) {
            auto typeArgs = r->typeArgs | std::views::transform([&](auto arg) {
                                return (IRType*)dispatch(arg);
                            })
                | range_utils::to_vector;

            return getConstraintInstantiation(
                r->getInstantiatedFrom(), typeArgs
            );
        })
        | range_utils::to_vector;

    instantiation->setLocation(function->getLocation(SourceRef()));
    instantiation->setNoMangle(function->getNoMangle(false));
    instantiation->setExtern(function->getExtern(false));

    // We can't add the function instantiation to the parent env right here,
    // because we don't yet know if the instantiation is legal. If it is not, we
    // are going to get errors later on.

    // function->parent->getEnv()->addFunctionInstantiation(
    //     function, instantiation
    // );

    m_irCtx = prevIrCtx;
    return instantiation;
}

IRType* Instantiator::instantiateType(
    IRType* type, Environment* env, GraphContext* irCtx
)
{
    auto prevEnv = m_env;
    auto prevIrCtx = m_irCtx;
    m_env = env;
    m_irCtx = irCtx;

    auto instantiation = (IRType*)dispatch(type);

    m_env = prevEnv;
    m_irCtx = prevIrCtx;
    return instantiation;
}

IRNode* Instantiator::visit(IRConstraintFunction* node)
{
    auto params = node->params | std::views::transform([&](auto const& p) {
                      return std::pair(p.first, (IRType*)dispatch(p.second));
                  })
        | range_utils::to_vector;

    auto result = (IRType*)dispatch(node->result);

    return m_irCtx->make(IRConstraintFunction(node->name, params, result));
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