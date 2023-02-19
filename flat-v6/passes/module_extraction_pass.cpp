#include "module_extraction_pass.hpp"

void ModuleExtractionPass::process(ASTSourceFile* node)
{
}

ModuleContext* ModuleExtractionPass::visit(ASTSourceFile* node)
{
    std::string name;
    for (auto const& segment : node->modulePath)
        name += ((name.empty()) ? "" : ".") + segment;

    if (!m_compCtx.getModule(name))
        m_compCtx.addModule(m_memCtx.make(ModuleContext(m_compCtx, name)));
    auto modCtx = m_compCtx.getModule(name);

    for (auto const& importPath : node->importPaths)
    {
        std::string importName;
        for (auto const& segment : importPath)
            importName += ((importName.empty()) ? "" : ".") + segment;
        if (!modCtx->imports.contains(importName))
            modCtx->imports.emplace(importName);
    }
}