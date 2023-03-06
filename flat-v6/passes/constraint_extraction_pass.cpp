#include "constraint_extraction_pass.hpp"

#include "../compiler.hpp"
#include "../ir/ir.hpp"

void ConstraintExtractionPass::process(ASTSourceFile* sourceFile)
{
    return dispatch(sourceFile);
}

void ConstraintExtractionPass::visit(ASTConstraintDeclaration* node)
{
    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    auto constraint =
        m_irCtx->make(IRConstraintTemplate(node->name, typeParams, {}, {}));

    constraint->setParent(m_module);
    constraint->setLocation(node->location);
    node->setIRConstraint(constraint);

    if (!m_module->getEnv()->addConstraint(constraint))
    {
        return m_logger.error(
            node->location,
            "Constraint " + node->name + " is already defined in module "
                + m_module->name
        );
    }
}

void ConstraintExtractionPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);
}