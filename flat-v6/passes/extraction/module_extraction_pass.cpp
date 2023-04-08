#include "module_extraction_pass.hpp"

#include <set>

void ModuleExtractionPass::process(ASTSourceFile* node)
{
    return dispatch(node);
}

void ModuleExtractionPass::visit(ASTSourceFile* node)
{
    std::string name;
    for (auto const& segment : node->modulePath)
        name += ((name.empty()) ? "" : ".") + segment;

    if (!m_compCtx.getModule(name))
    {
        auto mod =
            m_compCtx.addModule(m_irCtx.make(IRModule(name, {}, {}, {}, {})));
        mod->setIrCtx(m_irCtx.make(GraphContext()));
        mod->setEnv(mod->getIrCtx()->make(Environment(name, &m_compCtx)));
    }
    auto mod = m_compCtx.getModule(name);
    node->setIRModule(mod);
}