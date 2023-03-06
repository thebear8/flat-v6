#include "constraint_population_pass.hpp"

#include "../environment.hpp"
#include "../ir/ir.hpp"
#include "../util/error_logger.hpp"
#include "../util/graph_context.hpp"

void ConstraintPopulationPass::visit(ASTConstraintDeclaration* node)
{
    m_env = &Environment(node->name, m_module->getEnv());

    auto constraint = node->getIRConstraint();

    for (auto const& [name, typeArgs] : node->requirements)
    {
        auto requirement = m_env->findConstraint(name);
        if (!requirement)
        {
            return m_logger.error(
                node->location, "No constraint named " + name
            );
        }

        if (requirement->typeParams.size() != typeArgs.size())
        {
            return m_logger.error(
                node->location,
                "Number of type args for constraint " + name
                    + " does not match number of type params"
            );
        }

        std::vector<IRType*> irTypeArgs;
        for (auto typeArg : typeArgs)
        {
            auto&& [irType, error] =
                m_resolver.resolve(typeArg, m_env, m_irCtx);
            if (!irType)
                return m_logger.error(typeArg->location, error);

            irTypeArgs.push_back(irType);
        }

        auto requirementInstantiation =
            m_env->findConstraintInstantiation(requirement, irTypeArgs);

        if (!requirementInstantiation)
        {
            requirementInstantiation = m_env->addConstraintInstantiation(
                requirement,
                m_instantiator.makeConstraintInstantiation(
                    requirement, irTypeArgs
                )
            );
        }

        if (constraint->requirements.contains(requirementInstantiation))
        {
            return m_logger.error(
                node->location, "Requirement of same type already exists"
            );
        }

        constraint->requirements.emplace(requirementInstantiation);
    }

    m_env = nullptr;
}

void ConstraintPopulationPass::process(ASTSourceFile* node)
{
    return dispatch(node);
}

void ConstraintPopulationPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}