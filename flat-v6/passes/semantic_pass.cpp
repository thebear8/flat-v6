#include "semantic_pass.hpp"

#include <cassert>
#include <ranges>
#include <sstream>

#include "../util/zip_view.hpp"

void SemanticPass::process(IRModule* mod)
{
    dispatch(mod);
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

    for (auto const& [name, value] : node->fields)
        dispatch(value);

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
        auto error = m_env->inferTypeArgsAndValidate(
            fieldType, value->getType(), typeArgs, true
        );

        if (error.has_value())
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Value type for field " + name + " ("
                    + value->getType()->toString()
                    + ") is incompatible with struct field type ("
                    + fieldType->toString() + "): " + error.value(),
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
        m_module->getEnv()->getStructInstantiation(structTemplate, typeArgList);

    if (!instantiation)
    {
        instantiation =
            m_instantiator.makeStructInstantiation(structTemplate, typeArgList);
    }

    node->setType(instantiation);
    return node->getType();
}

IRType* SemanticPass::visit(IRUnaryExpression* node)
{
    auto value = dispatch(node->expression);
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
            unaryOperators.at(node->operation).name, {}, args, error
        );

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRBinaryExpression* node)
{
    auto left = dispatch(node->left);
    auto right = dispatch(node->right);

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
            binaryOperators.at(node->operation).name, {}, args, error
        );

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRCallExpression* node)
{
    std::vector<IRType*> args;
    for (auto arg : node->args)
        args.push_back(dispatch(arg));

    if (auto identifierExpression =
            dynamic_cast<IRIdentifierExpression*>(node->expression))
    {
        std::string error;
        auto target =
            findCallTarget(identifierExpression->value, {}, args, error);

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
    else
    {
        args.insert(args.begin(), dispatch(node->expression));

        std::string error;
        auto target = findCallTarget("__call__", {}, args, error);

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRIndexExpression* node)
{
    std::vector<IRType*> args;
    for (auto arg : node->args)
        args.push_back(dispatch(arg));

    auto value = dispatch(node->expression);
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
        auto target = findCallTarget("__call__", {}, args, error);

        if (!target)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function: " + error,
                nullptr
            );
        }

        node->setTarget(target);
        node->setType(target->result);
        return node->getType();
    }
}

IRType* SemanticPass::visit(IRFieldExpression* node)
{
    auto value = dispatch(node->expression);
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
    for (auto& statement : node->statements)
        dispatch(statement);

    return nullptr;
}

IRType* SemanticPass::visit(IRExpressionStatement* node)
{
    dispatch(node->expression);
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

        if (dispatch(value)->isVoidType())
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
    m_result = dispatch(node->expression);
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
    auto condition = dispatch(node->condition);
    if (!condition->isBoolType())
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "While condition has to be of boolean type",
            nullptr
        );
    }

    auto prevResult = m_result;
    dispatch(node->body);
    m_result = prevResult;

    return nullptr;
}

IRType* SemanticPass::visit(IRIfStatement* node)
{
    auto condition = dispatch(node->condition);
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
    dispatch(node->ifBody);
    auto ifResult = m_result;

    m_result = nullptr;
    if (node->elseBody)
        dispatch(node->elseBody);
    auto elseResult = m_result;

    m_result =
        (((ifResult != nullptr) && (elseResult != nullptr)) ? ifResult
                                                            : prevResult);

    return nullptr;
}

IRType* SemanticPass::visit(IRFunctionTemplate* node)
{
    if (!node->body)
        return nullptr;

    m_env = m_irCtx->make(Environment(node->name, m_env));

    for (auto typeParam : node->typeParams)
        m_env->addTypeParam(typeParam);

    for (auto requirement : node->requirements)
    {
        for (auto condition : requirement->conditions)
        {
            if (!m_env->addConstraintCondition(condition))
            {
                return m_logger.error(
                    node->getLocation(SourceRef()),
                    "Function " + formatFunctionHeadDescriptor(condition)
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

    dispatch(node->body);

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

IRType* SemanticPass::visit(IRModule* node)
{
    for (auto& decl : node->functions)
        dispatch(decl);
    return nullptr;
}

IRFunctionInstantiation* SemanticPass::findCallTarget(
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    std::string& error
)
{
    std::unordered_multimap<IRFunctionTemplate*, std::string> candidates;

    for (auto [i, iEnd] =
             m_module->getEnv()->getFunctionTemplateMap().equal_range(name);
         i != iEnd;
         ++i)
    {
        auto functionTemplate = i->second;
        auto const& functionTemplateTypeParams = i->second->typeParams;
        auto const& functionTemplateParams = i->second->params;

        if (args.size() != functionTemplateParams.size()
            || typeArgs.size() > functionTemplateTypeParams.size())
        {
            continue;
        }

        for (auto [j, jEnd] =
                 m_module->getEnv()->getFunctionInstantiationMap().equal_range(
                     i->second
                 );
             j != jEnd;
             ++j)
        {
            auto functionInstantiation = j->second;
            auto const& functionInstantiationTypeArgs = j->second->typeArgs;
            auto const& functionInstantiationParams = j->second->params;

            auto zippedTypeArgs = zip_view(
                std::views::all(functionInstantiationTypeArgs),
                std::views::all(typeArgs)
            );

            bool differingTypeArgs = false;
            for (auto [a, b] : zippedTypeArgs)
                differingTypeArgs = differingTypeArgs || (a != b);

            if (differingTypeArgs)
                continue;

            auto zippedArgs = zip_view(
                functionInstantiationParams | std::views::values,
                std::views::all(args)
            );

            bool differingArgs;
            for (auto [a, b] : zippedArgs)
                differingArgs = differingArgs || (a != b);

            if (differingArgs)
                continue;

            return functionInstantiation;
        }

        auto zippedTypeArgs = zip_view(
            std::views::all(functionTemplate->typeParams),
            std::views::all(typeArgs)
        );

        std::unordered_map typeArgMap(
            zippedTypeArgs.begin(), zippedTypeArgs.end()
        );

        auto zippedArgs = zip_view(
            functionTemplate->params | std::views::values, std::views::all(args)
        );

        bool incompatibleArgs = false;
        for (auto [param, arg] : zippedArgs)
        {
            incompatibleArgs = incompatibleArgs
                || !m_env->inferTypeArgsAndMatch(param, arg, typeArgMap, false);
        }

        if (incompatibleArgs)
            continue;

        std::vector<IRType*> typeArgList;
        for (auto typeParam : functionTemplate->typeParams)
        {
            if (!typeArgMap.contains(typeParam))
            {
                candidates.emplace(
                    functionTemplate,
                    "Could not infer value for type parameter "
                        + typeParam->toString() + " for call to "
                        + formatCallDescriptor(name, typeArgs, args)
                );
            }

            typeArgList.push_back(typeArgMap.at(typeParam));
        }

        if (!candidates.contains(functionTemplate))
        {
            return m_instantiator.makeFunctionInstantiation(
                functionTemplate, typeArgList
            );
        }
    }

    error =
        "No target for call to " + formatCallDescriptor(name, typeArgs, args);

    for (auto i = candidates.begin(), iEnd = candidates.end(); i != iEnd; ++i)
    {
        auto candidate = i->first;
        error +=
            "\n  Candidate: " + formatFunctionTemplateDescriptor(candidate);

        for (auto [j, jEnd] = candidates.equal_range(candidate); j != jEnd; ++j)
            error += "\n    " + j->second;
    }

    return nullptr;
}

std::string SemanticPass::formatFunctionHeadDescriptor(IRFunctionHead* value)
{
    std::stringstream descriptor;

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}

std::string SemanticPass::formatFunctionTemplateDescriptor(
    IRFunctionTemplate* value
)
{
    std::stringstream descriptor;

    std::string typeParams;
    for (auto typeParam : value->typeParams)
    {
        typeParams += (typeParams.empty() ? "" : ", ") + typeParam->toString();
    }

    if (!typeParams.empty())
        descriptor << "<" << typeParams << ">";

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}

std::string SemanticPass::formatFunctionInstantiationDescriptor(
    IRFunctionInstantiation* value
)
{
    std::stringstream descriptor;

    std::string typeArgs;
    auto zippedTypeArgs = zip_view(
        std::views::all(value->getInstantiatedFrom()->typeParams),
        std::views::all(value->typeArgs)
    );
    for (auto [typeParam, typeArg] : zippedTypeArgs)
    {
        typeArgs += (typeArgs.empty() ? "" : ", ") + typeParam->toString()
            + " = " + typeArg->toString();
    }

    if (!typeArgs.empty())
        descriptor << "<" << typeArgs << ">";

    std::string params;
    for (auto param : value->params)
    {
        params += (params.empty() ? "" : ", ") + param.first + ": "
            + param.second->toString();
    }

    descriptor << "(" << params << ")";
    return descriptor.str();
}

std::string SemanticPass::formatCallDescriptor(
    std::string targetName,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args
)
{
    std::stringstream descriptor;

    std::string typeArgString;
    for (auto typeArg : typeArgs)
    {
        typeArgString +=
            (typeArgString.empty() ? "" : ", ") + typeArg->toString();
    }

    if (!typeArgString.empty())
        descriptor << "<" << typeArgString << ">";

    std::string argString;
    for (auto arg : args)
    {
        argString += (argString.empty() ? "" : ", ") + arg->toString();
    }

    descriptor << "(" << argString << ")";
    return descriptor.str();
}