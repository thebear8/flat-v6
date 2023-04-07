#include "constraint_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../support/instantiator.hpp"

void ConstraintInstantiationUpdatePass::process(IRModule* node)
{
    return dispatch(node);
}

void ConstraintInstantiationUpdatePass::visit(IRModule* node)
{
    for (auto [constraintTemplate, constraintInstantiation] :
         node->getEnv()->getConstraintInstantiationMap())
    {
        m_instantiator.updateConstraintInstantiation(constraintInstantiation);
    }
}