#include "module_import_population_pass.hpp"

#include <set>

#include "../../compiler.hpp"

void ModuleImportPopulationPass::process(ASTSourceFile* node)
{
    return dispatch(node);
}

void ModuleImportPopulationPass::visit(ASTSourceFile* node)
{
    auto irModule = node->getIRModule();
    for (auto const& importPath : node->importPaths)
    {
        std::string importName;
        for (auto const& segment : importPath)
            importName += ((importName.empty()) ? "" : ".") + segment;

        auto importModule = m_compCtx.getModule(importName);
        if (!importModule)
        {
            return m_logger.error(
                node->location, "No module named " + importName
            );
        }

        if (!irModule->imports.contains(importModule))
            irModule->imports.emplace(importModule);
    }
}