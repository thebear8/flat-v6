#include "struct_extraction_pass.hpp"

#include <cassert>

void StructExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void StructExtractionPass::visit(ASTStructDeclaration* node)
{
    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    auto structTemplate =
        m_irCtx->make(IRStructTemplate(node->name, typeParams, {}));
    structTemplate->setParent(m_module);
    structTemplate->setLocation(node->location);
    m_module->structs.push_back(structTemplate);

    if (!m_module->getEnv()->addStruct(structTemplate))
    {
        return m_logger.error(
            node->location,
            "Struct " + node->name + " is already defined in module "
                + m_module->name
        );
    }

    node->setIRStruct(structTemplate);
}

void StructExtractionPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}