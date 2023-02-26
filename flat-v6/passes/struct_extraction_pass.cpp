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

    auto structType = m_irCtx->make(IRStructType(node->name, typeParams, {}));
    structType->setLocation(node->location);
    m_module->structs.push_back(structType);

    if (!m_env->addStruct(structType))
    {
        return m_logger.error(
            node->location,
            "Struct " + node->name + " is already defined in module "
                + m_module->name
        );
    }

    node->setIRStructType(structType);
}

void StructExtractionPass::visit(ASTSourceFile* node)
{
    std::string name;
    for (auto const& segment : node->modulePath)
        name += ((name.empty()) ? "" : ".") + segment;

    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();
    m_env = m_module->getEnv();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}