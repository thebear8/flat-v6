#include "constraint_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../support/constraint_instantiator.hpp"

void ConstraintInstantiationUpdatePass::process(IRModule* node)
{
    return dispatch(node);
}

void ConstraintInstantiationUpdatePass::visit(IRModule* node)
{
    for (auto [constraintTemplate, constraintInstantiation] :
         node->getEnv()->getConstraintInstantiationMap())
    {
        m_constraintInstantiator.updateConstraintInstantiation(
            constraintInstantiation
        );
    }
}