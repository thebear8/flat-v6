#include "function_extraction_pass.hpp"

#include "../../compiler.hpp"
#include "../../ir/ir.hpp"

void FunctionExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void FunctionExtractionPass::visit(ASTFunctionDeclaration* node)
{
    m_env = m_irCtx->make(Environment(node->name, m_module->getEnv()));

    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    for (auto p : typeParams)
        m_env->addTypeParam(p);

    std::vector<std::pair<std::string, IRType*>> params;
    for (auto const& [paramName, paramType] : node->parameters)
    {
        auto&& [irType, error] = m_resolver.resolve(paramType, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(paramType->location, error);

        params.push_back(std::make_pair(paramName, irType));
    }

    auto&& [result, error] = (node->result)
        ? (m_resolver.resolve(node->result, m_env, m_irCtx))
        : (std::make_tuple(m_compCtx.getVoid(), std::string()));

    if (!result)
        return m_logger.error(node->result->location, error);

    auto function = m_irCtx->make(
        IRFunctionTemplate(node->name, typeParams, params, result, {}, nullptr)
    );

    function->setParent(m_module);
    function->setLibraryNameForImport(node->lib);
    function->setLocation(node->location);
    node->setIRFunction(function);

    if (!m_module->getEnv()->addFunctionTemplate(function))
    {
        return m_logger.error(
            node->location,
            "Function " + node->name + " in module " + m_module->name
                + " is already defined"
        );
    }

    m_env = nullptr;
}

void FunctionExtractionPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}