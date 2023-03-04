#include "constraint_extraction_pass.hpp"

#include "../compiler.hpp"
#include "../ir/ir.hpp"
#include "support/ast_type_resolver.hpp"

void ConstraintExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void ConstraintExtractionPass::visit(ASTConstraintDeclaration* node)
{
    ASTTypeResolver resolver(m_env, m_irCtx);

    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    auto constraint =
        m_irCtx->make(IRConstraint(node->name, typeParams, {}, {}));

    constraint->setParent(m_module);
    constraint->setLocation(node->location);
    m_module->getEnv()->addConstraint(constraint);
}

void ConstraintExtractionPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();
    m_env = m_module->getEnv();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}