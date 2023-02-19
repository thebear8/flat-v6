#include "struct_extraction_pass.hpp"

void StructExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void StructExtractionPass::visit(ASTStructDeclaration* node)
{
    if (modCtx.getStruct(node->name))
        return logger.error(
            node->location,
            "Struct " + node->name + " is already defined in module "
                + modCtx.name
        );

    modCtx.addStruct(modCtx.irCtx.make(IRStructType(node->name, {})));
}

void StructExtractionPass::visit(ASTSourceFile* node)
{
    for (auto declaration : node->declarations)
        dispatch(declaration);
}