#include "semantic_pass.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <sstream>

#include "../../compiler.hpp"
#include "../../environment.hpp"
#include "../../support/formatter.hpp"
#include "../../util/assert.hpp"
#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"
#include "../support/call_target_resolver.hpp"
#include "../support/instantiator.hpp"
#include "../update/struct_instantiation_update_pass.hpp"

void SemanticPass::process(IRModule* node)
{
    m_module = node;
    m_irCtx = node->getIrCtx();

    for (auto& [name, function] : node->getEnv()->getFunctionMap())
        dispatchRef(function);

    m_module = nullptr;
    m_irCtx = nullptr;
}

IRType* SemanticPass::visit(IRIntegerExpression* node)
{
    node->setType(m_compCtx.getIntegerType(node->width, node->isSigned));
    return node->getType();
}

IRType* SemanticPass::visit(IRBoolExpression* node)
{
    node->setType(m_compCtx.getBool());
    return node->getType();
}

IRType* SemanticPass::visit(IRCharExpression* node)
{
    node->setType(m_compCtx.getChar());
    return node->getType();
}

IRType* SemanticPass::visit(IRStringExpression* node)
{
    node->setType(m_compCtx.getString());
    return node->getType();
}

IRType* SemanticPass::visit(IRIdentifierExpression* node)
{
    if (node->typeArgs.size() != 0)
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Variable Name cannot have type args",
            nullptr
        );
    }

    node->setType(m_env->findVariableType(node->value));
    if (!node->getType())
    {
        return m_logger.error(
            node->getLocation(SourceRef()), "Undefined Identifier", nullptr
        );
    }

    return node->getType();
}

IRType* SemanticPass::visit(IRStructExpression* node)
{
    auto structTemplate = m_env->findStruct(node->structName);
    if (!structTemplate)
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Undefined Struct Type " + node->structName,
            nullptr
        );
    }

    for (auto& [name, value] : node->fields)
        dispatchRef(value);

    for (auto const& [name, value] : node->fields)
    {
        if (!structTemplate->fields.contains(name))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Struct " + structTemplate->name + " does not contain field "
                    + name,
                nullptr
            );
        }
    }

    for (auto const& [fieldName, fieldType] : structTemplate->fields)
    {
        if (!node->fields.contains(fieldName))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No initializer for field " + fieldName + ": "
                    + fieldType->toString(),
                nullptr
            );
        }
    }

    if (node->typeArgs.size() > structTemplate->typeParams.size())
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Struct expression has more type args than type parameters exist",
            nullptr
        );
    }

    auto zippedTypeArgs = zip_view(
        std::views::all(structTemplate->typeParams),
        std::views::all(node->typeArgs)
    );
    std::unordered_map typeArgs(zippedTypeArgs.begin(), zippedTypeArgs.end());

    for (auto const& [name, value] : node->fields)
    {
        auto fieldType = structTemplate->fields.at(name);

        std::string error;
        if (!m_env->inferTypeArgsAndMatch(
                fieldType, value->getType(), typeArgs, true, error
            ))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Value type for field " + name + " ("
                    + value->getType()->toString()
                    + ") is incompatible with struct field type ("
                    + fieldType->toString() + "): " + error,
                nullptr
            );
        }
    }

    std::vector<IRType*> typeArgList;
    for (auto typeParam : structTemplate->typeParams)
    {
        if (!typeArgs.contains(typeParam))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Could not infer value for type parameter "
                    + typeParam->toString(),
                nullptr
            );
        }

        typeArgList.push_back(typeArgs.at(typeParam));
    }

    auto instantiation =
        m_instantiator.getStructInstantiation(structTemplate, typeArgList);
    m_structUpdatePass.update(instantiation);
    FLC_ASSERT(instantiation->fields.size() != 0);

    node->setType(instantiation);
    return node->getType();
}

IRType* SemanticPass::visit(IRLoweredCallExpression* node, IRNode*& ref)
{
    std::vector<IRType*> args;
    for (auto& arg : node->args)
        args.push_back(dispatchRef(arg));

    std::string error;
    auto target =
        findCallTarget(node->name, node->typeArgs, args, nullptr, error);

    if (!target)
        return m_logger.error(node->getLocation(SourceRef()), error, nullptr);

    auto bound = m_irCtx->make(IRBoundCallExpression(target, node->args));
    bound->setLocation(node->getLocation(SourceRef()));
    bound->setType(target->result);

    ref = bound;
    return bound->getType();
}

IRType* SemanticPass::visit(IRFieldExpression* node)
{
    auto value = dispatchRef(node->expression);
    if (!value->isStructType())
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Left side of field expression has to be of struct type",
            nullptr
        );
    }

    auto structType = (IRStruct*)value;
    if (!structType->fields.contains(node->fieldName))
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Struct " + structType->name + " does not have a field named "
                + node->fieldName,
            nullptr
        );
    }

    node->setType(structType->fields.at(node->fieldName));
    return node->getType();
}

IRType* SemanticPass::visit(IRBlockStatement* node)
{
    m_env = m_envCtx.make(
        Environment("BlockStatement@" + std::to_string((size_t)node), m_env)
    );

    for (auto& statement : node->statements)
        dispatchRef(statement);

    m_env = m_env->getParent();
    return nullptr;
}

IRType* SemanticPass::visit(IRExpressionStatement* node)
{
    dispatchRef(node->expression);
    return nullptr;
}

IRType* SemanticPass::visit(IRVariableStatement* node)
{
    for (auto& [name, value] : node->items)
    {
        if (m_env->getVariableType(name))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Variable is already defined",
                nullptr
            );
        }

        if (dispatchRef(value)->isVoidType())
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Variable cannot have void type",
                nullptr
            );
        }

        m_env->addVariableType(name, value->getType());
    }

    return nullptr;
}

IRType* SemanticPass::visit(IRReturnStatement* node)
{
    m_result = dispatchRef(node->expression);
    if (m_result != m_expectedResult)
    {
        m_result = nullptr;
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Return expression has to be of function result type",
            nullptr
        );
    }

    return nullptr;
}

IRType* SemanticPass::visit(IRWhileStatement* node)
{
    auto condition = dispatchRef(node->condition);
    if (!condition->isBoolType())
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "While condition has to be of boolean type",
            nullptr
        );
    }

    auto prevResult = m_result;
    dispatchRef(node->body);
    m_result = prevResult;

    return nullptr;
}

IRType* SemanticPass::visit(IRIfStatement* node)
{
    auto condition = dispatchRef(node->condition);
    if (!condition->isBoolType())
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "If condition has to be of boolean type",
            nullptr
        );
    }

    auto prevResult = m_result;

    m_result = nullptr;
    dispatchRef(node->ifBody);
    auto ifResult = m_result;

    m_result = nullptr;
    if (node->elseBody)
        dispatchRef(node->elseBody);
    auto elseResult = m_result;

    m_result =
        (((ifResult != nullptr) && (elseResult != nullptr)) ? ifResult
                                                            : prevResult);

    return nullptr;
}

IRType* SemanticPass::visit(IRNormalFunction* node)
{
    if (!node->body)
        return nullptr;

    m_env = m_envCtx.make(Environment(node->name, m_module->getEnv()));

    for (auto typeParam : node->typeParams)
        m_env->addTypeParam(typeParam);

    for (auto requirement : node->requirements)
    {
        for (auto condition : requirement->conditions)
        {
            if (!m_env->addFunction(condition))
            {
                return m_logger.error(
                    node->getLocation(SourceRef()),
                    "Function "
                        + m_formatter.formatFunctionDescriptor(condition)
                        + " is already defined",
                    nullptr
                );
            }
        }
    }

    for (auto& [paramName, paramType] : node->params)
        m_env->addVariableType(paramName, paramType);

    m_result = nullptr;
    m_expectedResult = node->result;

    dispatchRef(node->body);

    m_env = m_env->getParent();

    if (!m_expectedResult->isVoidType() && !m_result)
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Missing return statement in function " + node->name
                + ", should return " + m_expectedResult->toString(),
            nullptr
        );
    }

    return nullptr;
}

IRFunction* SemanticPass::findCallTarget(
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    IRType* result,
    optional_ref<std::string> reason
)
{
    auto functions = m_callTargetResolver.findMatchingFunctions(
        m_env, name, typeArgs, args, result
    );

    if (functions.size() == 1)
        return functions.front();

    // TODO: generate better/more precise diagnostic messages
    if (functions.size() == 0)
    {
        if (reason.has_value())
        {
            reason = "No matching function for call to "
                + m_formatter.formatCallDescriptor(
                    name, typeArgs, args, result
                );
        }
    }
    else
    {
        if (reason.has_value())
            reason = "Multiple matching functions for call to "
                + m_formatter.formatCallDescriptor(
                    name, typeArgs, args, result
                );
    }

    return nullptr;
}