#include "function_instantiation_update_pass.hpp"

#include "../../environment.hpp"
#include "../support/function_body_instantiator.hpp"
#include "../support/function_instantiator.hpp"

void FunctionInstantiationUpdatePass::process(IRModule* node)
{
    return dispatch(node);
}

void FunctionInstantiationUpdatePass::visit(IRModule* node)
{
    for (auto [functionTemplate, functionInstantiation] :
         node->getEnv()->getFunctionInstantiationMap())
    {
        m_functionInstantiator.updateRequirements(functionInstantiation);
        m_functionBodyInstantiator.updateBody(functionInstantiation);
    }
}