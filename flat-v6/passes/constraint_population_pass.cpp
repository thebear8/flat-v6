#include "constraint_population_pass.hpp"

#include "../compiler.hpp"
#include "../environment.hpp"
#include "../ir/ir.hpp"
#include "../util/error_logger.hpp"
#include "../util/graph_context.hpp"

void ConstraintPopulationPass::process(ASTSourceFile* node)
{
    dispatch(node);
}

IRNode* ConstraintPopulationPass::visit(ASTRequirement* node)
{
    auto constraint = m_env->findConstraint(node->constraintName);

    if (!constraint)
    {
        return m_logger.error(
            node->location,
            "No constraint named " + node->constraintName,
            nullptr
        );
    }

    if (node->typeArgs.size() != constraint->typeParams.size())
    {
        return m_logger.error(
            node->location,
            "Number of type args does not match number of type parameters",
            nullptr
        );
    }

    std::vector<IRType*> typeArgs;
    for (auto typeArg : node->typeArgs)
    {
        auto&& [irType, error] = m_resolver.resolve(typeArg, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(typeArg->location, error, nullptr);

        typeArgs.push_back(irType);
    }

    auto instantiation =
        m_env->findConstraintInstantiation(constraint, typeArgs);

    if (!instantiation)
    {
        instantiation = m_env->addConstraintInstantiation(
            constraint,
            m_instantiator.makeConstraintInstantiation(constraint, typeArgs)
        );
    }

    return instantiation;
}

IRNode* ConstraintPopulationPass::visit(ASTConstraintCondition* node)
{
    std::vector<std::pair<std::string, IRType*>> params;
    for (auto const& [name, type] : node->params)
    {
        auto&& [irType, error] = m_resolver.resolve(type, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(node->location, error, nullptr);

        params.push_back(std::make_pair(name, irType));
    }

    auto&& [result, error] = (node->result)
        ? (m_resolver.resolve(node->result, m_env, m_irCtx))
        : (std::make_tuple(m_compCtx.getVoid(), std::string()));

    if (!result)
        return m_logger.error(node->location, error, nullptr);

    auto condition =
        m_irCtx->make(IRConstraintCondition(node->functionName, params, result)
        );

    condition->setLocation(node->location);
    return condition;
}

IRNode* ConstraintPopulationPass::visit(ASTConstraintDeclaration* node)
{
    m_env = &Environment(node->name, m_module->getEnv());

    auto constraint = node->getIRConstraint();

    for (auto typeParam : constraint->typeParams)
        m_env->addTypeParam(typeParam);

    std::vector<IRConstraintInstantiation*> requirements;
    for (auto requirement : node->requirements)
    {
        auto instantiation = (IRConstraintInstantiation*)dispatch(requirement);

        if (constraint->requirements.contains(instantiation))
        {
            return m_logger.error(
                requirement->location,
                "Requirement of same type already exists",
                nullptr
            );
        }

        constraint->requirements.emplace(instantiation);
    }

    std::vector<IRConstraintCondition*> conditions;
    for (auto condition : node->conditions)
        conditions.push_back((IRConstraintCondition*)dispatch(condition));

    m_env = nullptr;
}

IRNode* ConstraintPopulationPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}