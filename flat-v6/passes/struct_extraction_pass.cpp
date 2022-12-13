#include "struct_extraction_pass.hpp"

void StructExtractionPass::visit(ASTStructDeclaration* node)
{
	assert(moduleCtx && "moduleCtx cannot be null");

	if (modCtx.structTypes.contains(node->name))
		return logger.error(node, "Struct " + node->name + " is already defined in module " + moduleCtx->name);

	modCtx.getStructType(node->name);
}

void StructExtractionPass::visit(ASTSourceFile* node)
{
	std::string name;
	for (auto const& segment : node->modulePath)
		name += ((name.empty()) ? "" : ".") + segment;

	for (auto const& importPath : node->importPaths)
	{
		std::string importName;
		for (auto const& segment : importPath)
			importName += ((importName.empty()) ? "" : ".") + segment;
		if (!modCtx.imports.contains(importName))
			modCtx.imports.emplace(importName);
	}

	for (auto declaration : node->declarations)
		dispatch(declaration);
}