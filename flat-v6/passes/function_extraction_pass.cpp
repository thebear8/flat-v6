#include "function_extraction_pass.hpp"

void FunctionExtractionPass::process(IRSourceFile* sourceFile)
{
	return dispatch(sourceFile);
}

void FunctionExtractionPass::visit(IRFunctionDeclaration* node)
{
	if (!modCtx.addFunction(node))
		return logger.error("Function " + node->name + " in module " + modCtx.name + " is already defined");
}

void FunctionExtractionPass::visit(IRSourceFile* node)
{
	for (auto decl : node->declarations)
		dispatch(decl);
}