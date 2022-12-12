#include "struct_extraction_pass.hpp"

void StructExtractionPass::visit(ASTStructDeclaration* node)
{
	if (moduleCtx->structTypes.contains(node->name))
		return logger.error(node, "Struct " + node->name + " is already defined in module " + moduleCtx->name);

	moduleCtx->structTypes.try_emplace(node->name, new StructType(compilationCtx.typeCtx, node->name));
}

void StructExtractionPass::visit(ASTSourceFile* node)
{
	std::string name;
	for (auto const& segment : node->modulePath)
		name += ((name.empty()) ? "" : ".") + segment;

	moduleCtx = compilationCtx.getModule(name);
	for (auto const& importPath : node->importPaths)
	{
		std::string importName;
		for (auto const& segment : importPath)
			importName += ((importName.empty()) ? "" : ".") + segment;
		if (!moduleCtx->imports.contains(importName))
			moduleCtx->imports.emplace(importName);
	}

	for (auto declaration : node->declarations)
		dispatch(declaration);
}