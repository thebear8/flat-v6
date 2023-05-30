#include "function_extraction_pass.hpp"

#include "../../compiler.hpp"
#include "../../ir/ir.hpp"

void FunctionExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void FunctionExtractionPass::visit(ASTFunctionDeclaration* node)
{
    m_env = m_envCtx.make(Environment(node->name, m_module->getEnv()));

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

    auto function = m_irCtx->make(IRNormalFunction(
        m_module,
        nullptr,
        node->name,
        typeParams,
        {},
        params,
        result,
        {},
        nullptr
    ));

    node->setIRFunction(function);
    function->setLocation(node->location);

    for (auto attribute : node->attributes)
    {
        if (attribute->name == "no_mangle")
            function->setNoMangle(true);
        else if (attribute->name == "extern")
            function->setExtern(true);
        else if (attribute->name == "test")
            (void)0;
        else
            FLC_ASSERT(false);
    }

    if (!m_module->getEnv()->addFunction(function))
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