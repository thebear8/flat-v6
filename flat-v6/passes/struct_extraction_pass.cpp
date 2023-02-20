#include "struct_extraction_pass.hpp"

void StructExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void StructExtractionPass::visit(ASTStructDeclaration* node)
{
    if (m_env->getStruct(node->name))
        return m_logger.error(
            node->location,
            "Struct " + node->name + " is already defined in module "
                + m_module->name
        );

    auto structType = m_irCtx->make(IRStructType(node->name, {}, {}, {}));
    m_module->structs.push_back(structType);
    m_env->addStruct(structType);
}

void StructExtractionPass::visit(ASTSourceFile* node)
{
    std::string name;
    for (auto const& segment : node->modulePath)
        name += ((name.empty()) ? "" : ".") + segment;

    m_module = m_compCtx.getModule(name);
    assert(
        m_module
        && "Module has to exist, should be created by ModuleExtractionPass"
    );
    m_irCtx = m_module->getMD<GraphContext*>().value();
    m_env = m_module->getMD<Environment*>().value();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}