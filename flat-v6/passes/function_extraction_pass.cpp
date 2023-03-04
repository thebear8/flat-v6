#include "function_extraction_pass.hpp"

#include "../compiler.hpp"
#include "../ir/ir.hpp"
#include "support/ast_type_resolver.hpp"

void FunctionExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void FunctionExtractionPass::visit(ASTFunctionDeclaration* node)
{
    ASTTypeResolver resolver(m_env, m_irCtx);

    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    std::vector<std::pair<std::string, IRType*>> params;
    for (auto const& [name, type] : node->parameters)
    {
        auto&& [irType, error] = resolver.getIRType(type);
        if (!irType)
        {
            return m_logger.error(type->location, error);
        }
        params.push_back(std::make_pair(name, irType));
    }

    auto&& [result, error] = resolver.getIRType(node->result);

    auto function = m_irCtx->make(
        IRFunctionTemplate(node->name, typeParams, params, result, {}, nullptr)
    );

    function->setParent(m_module);
    function->setLibraryNameForImport(node->lib);
    function->setLocation(node->location);
    m_module->getEnv()->addFunction(function);
}

void FunctionExtractionPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();
    m_env = m_module->getEnv();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}