#include "struct_population_pass.hpp"

#include "../../environment.hpp"
#include "../../ir/ir.hpp"
#include "../../util/error_logger.hpp"
#include "../../util/graph_context.hpp"

void StructPopulationPass::visit(ASTStructDeclaration* node)
{
    m_env = m_irCtx->make(Environment(node->name, m_module->getEnv()));

    auto structTemplate = node->getIRStruct();

    for (auto typeParam : structTemplate->typeParams)
        m_env->addTypeParam(typeParam);

    for (auto const& [name, type] : node->fields)
    {
        if (structTemplate->fields.contains(name))
        {
            return m_logger.error(
                node->location,
                "Field " + name + " of struct " + node->name
                    + " has multiple definitions"
            );
        }

        auto [irType, error] = m_resolver.resolve(type, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(node->location, error);

        structTemplate->fields.try_emplace(name, irType);
    }

    m_env = nullptr;
}

void StructPopulationPass::process(ASTSourceFile* node)
{
    return dispatch(node);
}

void StructPopulationPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}