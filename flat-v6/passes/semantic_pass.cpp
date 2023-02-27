#include "semantic_pass.hpp"

#include <cassert>
#include <ranges>

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
    auto structType = m_env->findStruct(node->structName);
    if (!structType)
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
        if (!structType->fields.contains(name))
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Struct " + structType->name + " does not contain field "
                    + name,
                nullptr
            );
        }
    }

    for (auto const& [fieldName, fieldType] : structType->fields)
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

    if (node->typeArgs.size() > structType->typeParams.size())
    {
        return m_logger.error(
            node->getLocation(SourceRef()),
            "Struct expression has more type args than type parameters exist",
            nullptr
        );
    }

    std::unordered_map<IRGenericType*, IRType*> typeArgs;
    for (size_t i = 0; i < node->typeArgs.size(); i++)
    {
        typeArgs.try_emplace(
            structType->typeParams.at(i), node->typeArgs.at(i)
        );
    }

    for (auto const& [name, value] : node->fields)
    {
        auto fieldType = structType->fields.at(name);
        auto result =
            inferTypeArgsAndValidate(fieldType, value->getType(), typeArgs);
        if (result.has_value())
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Value type for field " + name + " ("
                    + value->getType()->toString()
                    + ") is incompatible with struct field type ("
                    + fieldType->toString() + "): " + result.value(),
                nullptr
            );
        }
    }

    std::vector<IRType*> typeArgList;
    for (auto typeParam : structType->typeParams)
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

    node->setType(m_irCtx->make(IRStructInstantiation(structType, typeArgList))
    );
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
        auto function =
            m_env->findFunction(unaryOperators.at(node->operation).name, args);
        if (!function)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function "
                    + unaryOperators.at(node->operation).name + " for type "
                    + value->toString(),
                nullptr
            );
        }

        node->setTarget(function);
        node->setType(function->result);
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
        auto function =
            m_env->findFunction(binaryOperators.at(node->operation).name, args);
        if (!function)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function "
                    + binaryOperators.at(node->operation).name + " for types "
                    + left->toString() + ", " + right->toString(),
                nullptr
            );
        }

        if (binaryOperators.at(node->operation).category
                == OperatorCategory::BinaryAssign
            && function->result != left)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Assignment operator overload function has to return a value that has the type of the left operand",
                nullptr
            );
        }

        node->setTarget(function);
        node->setType(function->result);
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
        auto function = m_env->findFunction(identifierExpression->value, args);
        if (!function)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching function " + identifierExpression->value,
                nullptr
            );
        }

        node->setTarget(function);
        node->setType(function->result);
        return node->getType();
    }
    else
    {
        args.insert(args.begin(), dispatch(node->expression));
        auto function = m_env->findFunction("__call__", args);
        if (!function)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function __call__ for type "
                    + args.front()->toString(),
                nullptr
            );
        }

        node->setTarget(function);
        node->setType(function->result);
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
        auto function = m_env->findFunction("__index__", args);
        if (!function)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No matching operator function __index__ for type "
                    + args.front()->toString(),
                nullptr
            );
        }

        node->setTarget(function);
        node->setType(function->result);
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

    auto structType = (IRStructType*)value;
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

IRType* SemanticPass::visit(IRFunction* node)
{
    if (!node->body)
        return nullptr;

    m_env = m_irCtx->make(Environment(node->name, m_env));

    for (auto typeParam : node->typeParams)
        m_env->addTypeParam(typeParam);

    for (auto& [name, args] : node->requirements)
    {
        auto constraint = m_env->getConstraint(name);
        if (!constraint)
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "No constraint named " + name,
                nullptr
            );
        }

        if (args.size() != constraint->typeParams.size())
        {
            return m_logger.error(
                node->getLocation(SourceRef()),
                "Number of args does not match number of type parameters for constraint "
                    + constraint->name,
                nullptr
            );
        }

        std::unordered_map<IRType*, IRType*> typeParamToArgLookup;
        for (size_t i = 0; i < args.size(); i++)
        {
            typeParamToArgLookup.try_emplace(
                constraint->typeParams[i], args[i]
            );
        }

        for (auto condition : constraint->conditions)
        {
            assert(condition && "Condition cannot be nullptr");
            auto function = dynamic_cast<IRFunction*>(condition);
            if (!function)
            {
                return m_logger.error(
                    condition->getLocation(SourceRef()),
                    "Constraint condition has to be a function declaration",
                    nullptr
                );
            }

            auto params = function->params | std::views::transform([&](auto p) {
                              if (typeParamToArgLookup.contains(p.second))
                                  p.second = typeParamToArgLookup.at(p.second);
                              return p;
                          });

            m_env->addFunction(m_irCtx->make(IRFunction(
                function->name,
                function->typeParams,
                std::vector<std::pair<std::string, std::vector<IRType*>>>(),
                std::vector(params.begin(), params.end()),
                function->result,
                nullptr
            )));
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
