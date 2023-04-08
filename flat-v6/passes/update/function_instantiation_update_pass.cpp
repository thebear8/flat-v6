#include "function_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../support/instantiator.hpp"

void FunctionInstantiationUpdatePass::process(IRModule* node)
{
    return dispatch(node);
}

void FunctionInstantiationUpdatePass::visit(IRModule* node)
{
    for (auto [functionTemplate, functionInstantiation] :
         node->getEnv()->getFunctionInstantiationMap())
    {
        m_instantiator.updateFunctionInstantiation(functionInstantiation);
    }
}