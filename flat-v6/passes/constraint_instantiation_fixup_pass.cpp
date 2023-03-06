#include "constraint_instantiation_fixup_pass.hpp"

#include "../environment.hpp"

void ConstraintInstantiationFixupPass::process(IRModule* node)
{
    return dispatch(node);
}

void ConstraintInstantiationFixupPass::visit(IRModule* node)
{
    for (auto [constraintTemplate, constraintInstantiation] :
         node->getEnv()->getConstraintInstantiationMap())
    {
        m_instantiator.fixupConstraintInstantiation(constraintInstantiation);
    }
}