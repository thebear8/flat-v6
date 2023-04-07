#include "struct_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../support/instantiator.hpp"

void StructInstantiationUpdatePass::process(IRModule* node)
{
    return dispatch(node);
}

void StructInstantiationUpdatePass::visit(IRModule* node)
{
    for (auto [structTemplate, structInstantiation] :
         node->getEnv()->getStructInstantiationMap())
    {
        m_instantiator.updateStructInstantiation(structInstantiation);
    }
}