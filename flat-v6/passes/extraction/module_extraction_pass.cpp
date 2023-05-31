#include "module_extraction_pass.hpp"

#include <set>

#include "../../compiler.hpp"
#include "../../environment.hpp"
#include "../../util/graph_context.hpp"

void ModuleExtractionPass::process(ASTSourceFile* node)
{
    return dispatch(node);
}

void ModuleExtractionPass::visit(ASTSourceFile* node)
{
    if (!m_compCtx.getModule(node->modulePath))
    {
        auto mod = m_compCtx.addModule(
            m_irCtx.make(IRModule(node->modulePath, {}, {}, {}, {}))
        );
        mod->setIrCtx(m_irCtx.make(GraphContext()));
        mod->setEnv(mod->getIrCtx()->make(
            Environment(node->modulePath, m_compCtx.getBuiltins()->getEnv())
        ));
    }
    auto mod = m_compCtx.getModule(node->modulePath);
    node->setIRModule(mod);
}