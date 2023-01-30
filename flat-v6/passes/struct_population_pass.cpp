#include "struct_population_pass.hpp"

void StructPopulationPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void StructPopulationPass::visit(ASTStructDeclaration* node)
{
    auto structType = modCtx.getStruct(node->name);
    assert(structType && "structType cannot be null");

    for (auto const& [fieldName, fieldType] : node->fields)
        structType->fields.push_back(
            std::pair(fieldName, modCtx.getType(fieldType)));
}

void StructPopulationPass::visit(ASTSourceFile* node)
{
    for (auto declaration : node->declarations)
        dispatch(declaration);
}