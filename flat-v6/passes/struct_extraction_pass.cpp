#include "struct_extraction_pass.hpp"

void StructExtractionPass::process(ASTSourceFile* sourceFile)
{
	return dispatch(sourceFile);
}

void StructExtractionPass::visit(ASTStructDeclaration* node)
{
	if(!modCtx.createStruct(node->name))
		return logger.error(node, "Struct " + node->name + " is already defined in module " + modCtx.name);
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