#include "struct_instantiation_fixup_pass.hpp"

#include "../environment.hpp"

void StructInstantiationFixupPass::process(IRModule* node)
{
    return dispatch(node);
}

void StructInstantiationFixupPass::visit(IRModule* node)
{
    for (auto [structTemplate, structInstantiation] :
         node->getEnv()->getStructInstantiationMap())
    {
        m_instantiator.fixupStructInstantiationFields(structInstantiation);
    }
}