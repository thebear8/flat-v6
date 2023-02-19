#include "function_extraction_pass.hpp"

void FunctionExtractionPass::process(IRSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void FunctionExtractionPass::visit(IRFunctionDeclaration* node)
{
    if (!m_modCtx.addFunction(node))
        return m_logger.error(
            node->getMD<SourceRef>().value_or(SourceRef()),
            "Function " + node->name + " in module " + m_modCtx.name
                + " is already defined"
        );
}

void FunctionExtractionPass::visit(IRSourceFile* node)
{
    for (auto decl : node->declarations)
        dispatch(decl);
}