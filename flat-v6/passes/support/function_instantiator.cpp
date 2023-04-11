#include "function_instantiator.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <vector>

#include "../../compiler.hpp"
#include "../../ir/ir.hpp"
#include "../../util/assert.hpp"
#include "../../util/error_logger.hpp"
#include "../../util/graph_context.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "constraint_instantiator.hpp"
#include "struct_instantiator.hpp"

IRFunctionInstantiation* FunctionInstantiator::getFunctionInstantiation(
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
                      return std::pair(p.first, (IRType*)dispatch(p.second));
                  })
        | range_utils::to_vector;

    auto result = (IRType*)dispatch(functionTemplate->result);

    auto instantiation = m_irCtx->make(IRFunctionInstantiation(
        functionTemplate->name, typeArgs, params, result, {}, nullptr
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

IRFunctionInstantiation* FunctionInstantiator::updateRequirements(
    IRFunctionInstantiation* functionInstantiation
)
{
    auto functionTemplate = functionInstantiation->getInstantiatedFrom();

    FLC_ASSERT(
        functionInstantiation->typeArgs.size()
            == functionTemplate->typeParams.size(),
        "Number of type args has to match number of type params"
    );

    m_env = m_envCtx.make(Environment(
        functionTemplate->name, functionTemplate->getParent()->getEnv()
    ));
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

    m_env = nullptr;
    m_irCtx = nullptr;
    return functionInstantiation;
}

IRNode* FunctionInstantiator::visit(IRConstraintInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto a) {
                        return (IRType*)dispatch(a);
                    })
        | range_utils::to_vector;

    return m_constraintInstantiator.getConstraintInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* FunctionInstantiator::visit(IRStructInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return m_structInstantiator.getStructInstantiation(
        node->getInstantiatedFrom(), typeArgs
    );
}

IRNode* FunctionInstantiator::visit(IRFunctionInstantiation* node)
{
    auto typeArgs = node->typeArgs | std::views::transform([&](auto arg) {
                        return (IRType*)dispatch(arg);
                    })
        | range_utils::to_vector;

    return getFunctionInstantiation(node->getInstantiatedFrom(), typeArgs);
}

IRNode* FunctionInstantiator::visit(IRGenericType* node)
{
    if (auto v = m_env->findTypeParamValue(node))
        return v;
    return node;
}

IRNode* FunctionInstantiator::visit(IRPointerType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRPointerType(base));
}

IRNode* FunctionInstantiator::visit(IRArrayType* node)
{
    auto base = (IRType*)dispatch(node->base);
    if (base == node->base)
        return node;

    return m_irCtx->make(IRArrayType(base));
}