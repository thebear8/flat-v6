#include "struct_population_pass.hpp"

void StructPopulationPass::visit(ASTStructDeclaration* node)
{
	assert(moduleCtx && "moduleCtx cannot be null");

	auto structType = moduleCtx->getStructType(node->name);
	assert(structType && "structType cannot be null");

	for (auto const& [fieldName, fieldType] : node->fields)
		structType->addField(fieldName, moduleCtx->getType(fieldType));
}

void StructPopulationPass::visit(ASTSourceFile* node)
{
	std::string name;
	for (auto const& segment : node->modulePath)
		name += ((name.empty()) ? "" : ".") + segment;

	moduleCtx = compilationCtx.getModule(name);

	for (auto declaration : node->declarations)
		dispatch(declaration);
}