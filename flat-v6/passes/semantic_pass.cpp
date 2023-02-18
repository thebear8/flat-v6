#include "semantic_pass.hpp"

IRType* SemanticPass::visit(IRFunctionDeclaration* node) {
    if (!node->body)
        return nullptr;

    m_env = m_envCtx.make(Environment(node->name, m_env));

    for (auto& name : node->typeParams)
        m_env->addGeneric(m_genericCtx.make(IRGenericType(name)));

    for (auto& [name, args] : node->requirements) {
        auto constraint = m_env->getConstraint(name);
        if (!constraint)
            return logger.error(
                node->location, "No constraint named " + name, nullptr
            );

        for (auto condition : constraint->conditions) {
            assert(condition && "Condition cannot be nullptr");
            if (auto function =
                    dynamic_cast<IRFunctionDeclaration*>(condition)) {
                if (!m_env->addFunction(function))
                    return lo
            } else
                return logger.error(
                    condition->location,
                    "Constraint condition has to be a function declaration",
                    nullptr
                );
        }
    }

    for (auto& [paramName, paramType] : node->params)
        m_env->addVariableType(paramName, paramType);

    m_result = nullptr;
    m_expectedResult = node->result;

    dispatch(node->body);

    m_env = m_env->getParent();

    if (!m_expectedResult->isVoidType() && !m_result)
        return logger.error(
            node->location,
            "Missing return statement in function " + node->name
                + ", should return " + m_expectedResult->toString(),
            nullptr
        );

    return nullptr;
}

IRType* SemanticPass::visit(IRSourceFile* node) {
    for (auto& decl : node->declarations)
        dispatch(decl);
    return nullptr;
}