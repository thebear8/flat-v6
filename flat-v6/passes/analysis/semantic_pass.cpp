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

void SemanticPass::process(IRModule* mod)
{
    dispatch(mod);
}

IRType* SemanticPass::visit(IRIntegerExpression* node, IRNode*& ref)
{
    node->setType(m_compCtx.getIntegerType(node->width, node->isSigned));
    return node->getType();
}

IRType* SemanticPass::visit(IRBoolExpression* node, IRNode*& ref)
{
    node->setType(m_compCtx.getBool());
    return node->getType();
}

IRType* SemanticPass::visit(IRCharExpression* node, IRNode*& ref)
{
    node->setType(m_compCtx.getChar());
    return node->getType();
}

IRType* SemanticPass::visit(IRStringExpression* node, IRNode*& ref)
{
    node->setType(m_compCtx.getString());
    return node->getType();
}

IRType* SemanticPass::visit(IRIdentifierExpression* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRStructExpression* node, IRNode*& ref)
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

    node->setType(instantiation);
    return node->getType();
}

IRType* SemanticPass::visit(IRUnaryExpression* node, IRNode*& ref)
{
    auto value = dispatchRef(node->expression);
    if (unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryArithmetic
        && value->isIntegerType())
    {
        node->setType(value);
        return node->getType();
    }
    else if (
        unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryBitwise
        && value->isIntegerType())
    {
        node->setType(value);
        return node->getType();
    }
    else if (
        unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryLogic
        && value->isBoolType())
    {
        node->setType(m_compCtx.getBool());
        return node->getType();
    }
    else
    {
        auto args = std::vector({ value });

        std::string error;
        auto target = findCallTarget(
            unaryOperators.at(node->operation).name, {}, args, nullptr, error
        );

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        // node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRBinaryExpression* node, IRNode*& ref)
{
    auto left = dispatchRef(node->left);
    auto right = dispatchRef(node->right);

    if (binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryArithmetic
        && (left->isIntegerType() && right->isIntegerType()))
    {
        node->setType(
            ((left->getBitSize() >= right->getBitSize()) ? left : right)
        );
        return node->getType();
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryBitwise
        && (left->isIntegerType() && right->isIntegerType())
        && (left->getBitSize() == right->getBitSize()))
    {
        node->setType(left);
        return node->getType();
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryComparison
        && (left->isIntegerType() && right->isIntegerType()))
    {
        node->setType(m_compCtx.getBool());
        return node->getType();
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryLogic
        && (left->isBoolType() && right->isBoolType()))
    {
        node->setType(m_compCtx.getBool());
        return node->getType();
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryEquality
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        node->setType(m_compCtx.getBool());
        return node->getType();
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryAssign
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        if (!dynamic_cast<IRIdentifierExpression*>(node->left))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Left side of assignment has to be identifier",
                nullptr
            );
        }

        if ((left->isIntegerType() && right->isIntegerType())
            && (left->getBitSize() < right->getBitSize()))
        {
            m_logger.warning(
                node->getLocation(SourceRef()),
                "Narrowing conversion from " + left->toString() + " to "
                    + right->toString()
            );
        }

        node->setType(left);
        return node->getType();
    }
    else
    {
        auto args = std::vector({ left, right });

        std::string error;
        auto target = findCallTarget(
            binaryOperators.at(node->operation).name, {}, args, nullptr, error
        );

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        // node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRCallExpression* node, IRNode*& ref)
{
    std::vector<IRType*> args;
    for (auto arg : node->args)
        args.push_back(dispatchRef(arg));

    if (auto identifierExpression =
            dynamic_cast<IRIdentifierExpression*>(node->expression))
    {
        std::string error;
        auto target = findCallTarget(
            identifierExpression->value, {}, args, nullptr, error
        );

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        // node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
    else
    {
        args.insert(args.begin(), dispatchRef(node->expression));

        std::string error;
        auto target = findCallTarget("__call__", {}, args, nullptr, error);

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        // node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRIndexExpression* node, IRNode*& ref)
{
    std::vector<IRType*> args;
    for (auto arg : node->args)
        args.push_back(dispatchRef(arg));

    auto value = dispatchRef(node->expression);
    if (value->isArrayType() && args.size() == 1
        && args.front()->isIntegerType())
    {
        node->setType(dynamic_cast<IRArrayType*>(value)->base);
        return node->getType();
    }
    if (value->isStringType() && args.size() == 1
        && args.front()->isIntegerType())
    {
        node->setType(m_compCtx.getU8());
        return node->getType();
    }
    else
    {
        args.insert(args.begin(), value);

        std::string error;
        auto target = findCallTarget("__call__", {}, args, nullptr, error);

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        // node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRFieldExpression* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRBlockStatement* node, IRNode*& ref)
{
    m_env = m_envCtx.make(
        Environment("BlockStatement@" + std::to_string((size_t)node), m_env)
    );

    for (auto& statement : node->statements)
        dispatchRef(statement);

    m_env = m_env->getParent();
    return nullptr;
}

IRType* SemanticPass::visit(IRExpressionStatement* node, IRNode*& ref)
{
    dispatchRef(node->expression);
    return nullptr;
}

IRType* SemanticPass::visit(IRVariableStatement* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRReturnStatement* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRWhileStatement* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRIfStatement* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRNormalFunction* node, IRNode*& ref)
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

IRType* SemanticPass::visit(IRModule* node, IRNode*& ref)
{
    m_module = node;
    m_irCtx = node->getIrCtx();

    for (auto& [name, function] : node->getEnv()->getFunctionMap())
    {
        dispatchRef(function);
    }

    m_module = nullptr;
    m_irCtx = nullptr;
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
    std::vector<IRType*> inferredTypeArgs;
    // std::set<IRFunction*> argRejected;
    // std::set<IRFunction*> requirementRejected;
    auto functions = m_callTargetResolver.getMatchingFunctions(
        m_env, name, typeArgs, args, result, inferredTypeArgs
        // argRejected,
        // requirementRejected
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