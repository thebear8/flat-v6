#include "semantic_pass.hpp"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <sstream>

#include "../../util/to_vector.hpp"
#include "../../util/zip_view.hpp"

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
                    "Function "
                        + m_formatter.formatFunctionHeadDescriptor(condition)
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

IRFunctionHead* SemanticPass::findCallTarget(
    std::string const& name,
    std::vector<IRType*> const& typeArgs,
    std::vector<IRType*> const& args,
    optional_ref<std::string> reason
)
{
    auto constraintConditions = [&]() {
        auto [it, end] = m_env->getConstraintConditionMap().equal_range(name);
        auto conditions = std::ranges::subrange(it, end) | std::views::values;

        return conditions | std::views::filter([&](auto c) {
                   auto candidateParams = c->params | std::views::values;
                   return candidateParams.size() == args.size()
                       && std::ranges::equal(candidateParams, args);
               })
            | range_utils::to_vector;
    }();

    auto nameMatchingFunctionTemplates = [&]() {
        auto [it, end] = m_env->getFunctionTemplateMap().equal_range(name);
        return std::ranges::subrange(it, end) | std::views::values;
    }();

    auto paramCompatibleFunctionTemplates = [&]() {
        return nameMatchingFunctionTemplates | std::views::filter([&](auto f) {
                   zip_view zippedTypesArgs(f->typeParams, typeArgs);

                   std::unordered_map typeArgMap(
                       zippedTypesArgs.begin(), zippedTypesArgs.end()
                   );

                   zip_view zippedArgs(f->params | std::views::values, args);
                   for (auto [param, arg] : zippedArgs)
                   {
                       if (param != arg
                           || !m_env->inferTypeArgsAndMatch(
                               param, arg, typeArgMap, true
                           ))
                       {
                           return false;
                       }
                   }

                   for (auto typeParam : f->typeParams)
                   {
                       if (!typeArgMap.contains(typeParam))
                           return false;
                   }

                   return true;
               });
    }();

    auto requirementCompatibleFunctionTemplates = [&]() {
        return paramCompatibleFunctionTemplates
            | std::views::filter([&](auto f) {
                   // TODO: filter by fulfilled requirements
                   return true;
               });
    }();

    auto functionTemplates = [&]() {
        auto candidates =
            requirementCompatibleFunctionTemplates
            | std::views::transform([&](auto f) {
                  std::unordered_map<IRGenericType*, IRType*> typeArgMap;
                  zip_view zippedArgs(f->params | std::views::values, args);
                  for (auto [param, arg] : zippedArgs)
                      m_env->inferTypeArgsAndMatch(
                          param, arg, typeArgMap, true
                      );
                  return std::pair(f, typeArgMap.size() - typeArgs.size());
              })
            | range_utils::to_vector;

        std::ranges::sort(candidates, [](auto const& a, auto const& b) {
            return a.second > b.second;
        });

        return candidates | std::views::transform([](auto const& f) {
                   return f.first;
               });
    }();

    if (constraintConditions.size() == 1 && (functionTemplates.size() == 0))
    {
        return constraintConditions.front();
    }
    else if (constraintConditions.size() == 0 && (functionTemplates.size() > 0))
    {
        return functionTemplates.front();
    }
    else if (constraintConditions.size() == 0 && functionTemplates.size() == 0)
    {
        if (reason.has_value())
        {
            reason = "No matching target for function call "
                + m_formatter.formatCallDescriptor(name, typeArgs, args);
        }

        return nullptr;
    }
    else
    {
        if (reason.has_value())
        {
            reason = "Ambiguous function call "
                + m_formatter.formatCallDescriptor(name, typeArgs, args);

            for (auto c : constraintConditions)
            {
                reason = reason.get() + "\n  Candidate: Constraint condition "
                    + m_formatter.formatFunctionHeadDescriptor(c);
            }

            for (auto f : functionTemplates)
            {
                reason = reason.get() + "\n  Candidate: Function template "
                    + m_formatter.formatFunctionTemplateDescriptor(f);
            }
        }

        return nullptr;
    }
}

bool SemanticPass::isConstraintSatisfied(
    IRConstraintInstantiation* constraint, optional_ref<std::string> reason
)
{
    for (auto requirement : constraint->requirements)
    {
        if (!isConstraintSatisfied(requirement, reason))
        {
            if (reason.has_value())
            {
                reason = "Requirement "
                    + m_formatter.formatConstraintInstantiationDescriptor(
                        requirement
                    )
                    + " is not satisfied: " + reason.get();
            }

            return false;
        }
    }

    for (auto condition : constraint->conditions)
    {
        auto args =
            condition->params | std::views::values | range_utils::to_vector;
        if (!findCallTarget(condition->name, {}, args, reason))
        {
            if (reason.has_value())
            {
                reason = "No matching target for condition "
                    + m_formatter.formatConstraintCondition(condition) + ": "
                    + reason.get();
            }

            return false;
        }
    }

    return true;
}