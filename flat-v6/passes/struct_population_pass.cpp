#include "struct_population_pass.hpp"

void StructPopulationPass::visit(ASTStructDeclaration* node)
{
	assert(moduleCtx && "moduleCtx cannot be null");

	auto structType = modCtx.getStructType(node->name);
	assert(structType && "structType cannot be null");

	for (auto const& [fieldName, fieldType] : node->fields)
		structType->addField(fieldName, modCtx.getType(fieldType));
}

void StructPopulationPass::visit(ASTSourceFile* node)
{
	for (auto declaration : node->declarations)
		dispatch(declaration);
}