#include "semantic_pass.hpp"

#include <ranges>

void SemanticPass::analyze(IRSourceFile* program) {
    dispatch(program);
}

IRType* SemanticPass::visit(IRIntegerExpression* node) {
    return (node->type = m_compCtx.getIntegerType(node->width, node->isSigned));
}

IRType* SemanticPass::visit(IRBoolExpression* node) {
    return (node->type = m_compCtx.getBool());
}

IRType* SemanticPass::visit(IRCharExpression* node) {
    return (node->type = m_compCtx.getChar());
}

IRType* SemanticPass::visit(IRStringExpression* node) {
    return (node->type = m_compCtx.getString());
}

IRType* SemanticPass::visit(IRIdentifierExpression* node) {
    if (auto type = m_env->findVariableType(node->value))
        return (node->type = type);
    return m_logger.error(node->location, "Undefined Identifier", nullptr);
}

IRType* SemanticPass::visit(IRStructExpression* node) {
    auto structType = m_modCtx.findStruct(node->structName);
    if (!structType)
        return m_logger.error(
            node->location, "Undefined Struct Type " + node->structName, nullptr
        );

    for (auto& [name, value] : node->fields)
        dispatch(value);

    for (auto& [name, value] : node->fields) {
        for (int i = 0; i < structType->fields.size(); i++) {
            auto& [fieldName, fieldType] = structType->fields[i];
            if (fieldName == name) {
                if (fieldType != value->type)
                    return m_logger.error(
                        node->location,
                        "Field " + name + " has type " + fieldType->toString()
                            + ", value type is " + value->type->toString(),
                        nullptr
                    );
                break;
            }

            if (i == structType->fields.size() - 1)
                return m_logger.error(
                    node->location,
                    "Struct " + structType->name
                        + " does not contain a field called " + name,
                    nullptr
                );
        }
    }

    for (auto& [fieldName, fieldType] : structType->fields) {
        for (int i = 0; i < node->fields.size(); i++) {
            auto& [name, value] = node->fields[i];
            if (name == fieldName) {
                if (value->type != fieldType)
                    return m_logger.error(
                        node->location,
                        "Field " + name + " has type " + fieldType->toString()
                            + ", value type is " + value->type->toString(),
                        nullptr
                    );
                break;
            }

            if (i == node->fields.size() - 1)
                return m_logger.error(
                    node->location,
                    "No initializer for field " + fieldName + ": "
                        + fieldType->toString(),
                    nullptr
                );
        }
    }

    return (node->type = structType);
}

IRType* SemanticPass::visit(IRUnaryExpression* node) {
    auto value = dispatch(node->expression);
    if (unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryArithmetic
        && value->isIntegerType()) {
        return (node->type = value);
    }
    else if (
        unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryBitwise
        && value->isIntegerType())
    {
        return (node->type = value);
    }
    else if (
        unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryLogic
        && value->isBoolType())
    {
        return (node->type = m_compCtx.getBool());
    } else {
        auto args = std::vector({ value });
        auto function = m_modCtx.findFunction(
            unaryOperators.at(node->operation).name, args
        );
        if (!function)
            return m_logger.error(
                node->location,
                "No matching operator function "
                    + unaryOperators.at(node->operation).name + " for type "
                    + value->toString(),
                nullptr
            );

        node->target = function;
        return (node->type = function->result);
    }
}

IRType* SemanticPass::visit(IRBinaryExpression* node) {
    auto left = dispatch(node->left);
    auto right = dispatch(node->right);

    if (binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryArithmetic
        && (left->isIntegerType() && right->isIntegerType())) {
        return (
            node->type =
                ((left->getBitSize() >= right->getBitSize()) ? left : right)
        );
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryBitwise
        && (left->isIntegerType() && right->isIntegerType())
        && (left->getBitSize() == right->getBitSize()))
    {
        return (node->type = left);
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryComparison
        && (left->isIntegerType() && right->isIntegerType()))
    {
        return (node->type = m_compCtx.getBool());
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryLogic
        && (left->isBoolType() && right->isBoolType()))
    {
        return (node->type = m_compCtx.getBool());
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryEquality
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        return (node->type = m_compCtx.getBool());
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryAssign
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        if (!dynamic_cast<IRIdentifierExpression*>(node->left))
            return m_logger.error(
                node->location,
                "Left side of assignment has to be identifier",
                nullptr
            );

        if ((left->isIntegerType() && right->isIntegerType())
            && (left->getBitSize() < right->getBitSize()))
            m_logger.warning(
                node->location,
                "Narrowing conversion from " + left->toString() + " to "
                    + right->toString()
            );

        return (node->type = left);
    } else {
        auto args = std::vector({ left, right });
        auto function = m_modCtx.findFunction(
            binaryOperators.at(node->operation).name, args
        );
        if (!function)
            return m_logger.error(
                node->location,
                "No matching operator function "
                    + binaryOperators.at(node->operation).name + " for types "
                    + left->toString() + ", " + right->toString(),
                nullptr
            );

        if (binaryOperators.at(node->operation).category
                == OperatorCategory::BinaryAssign
            && function->result != left)
            return m_logger.error(
                node->location,
                "Assignment operator overload function has to return a value that has the type of the left operand",
                nullptr
            );

        node->target = function;
        return (node->type = function->result);
    }
}

IRType* SemanticPass::visit(IRCallExpression* node) {
    std::vector<IRType*> args;
    for (auto arg : node->args)
        args.push_back(dispatch(arg));

    if (auto identifierExpression =
            dynamic_cast<IRIdentifierExpression*>(node->expression)) {
        auto function =
            m_modCtx.findFunction(identifierExpression->value, args);
        if (!function)
            return m_logger.error(
                node->location,
                "No matching function " + identifierExpression->value,
                nullptr
            );

        node->target = function;
        return (node->type = function->result);
    } else {
        args.insert(args.begin(), dispatch(node->expression));
        auto function = m_modCtx.findFunction("__call__", args);
        if (!function)
            return m_logger.error(
                node->location,
                "No matching operator function __call__ for type "
                    + args.front()->toString(),
                nullptr
            );

        node->target = function;
        return (node->type = function->result);
    }
}

IRType* SemanticPass::visit(IRIndexExpression* node) {
    std::vector<IRType*> args;
    for (auto arg : node->args)
        args.push_back(dispatch(arg));

    auto value = dispatch(node->expression);
    if (value->isArrayType() && args.size() == 1
        && args.front()->isIntegerType()) {
        return (node->type = dynamic_cast<IRArrayType*>(value)->base);
    }
    if (value->isStringType() && args.size() == 1
        && args.front()->isIntegerType()) {
        return (node->type = m_compCtx.getU8());
    } else {
        args.insert(args.begin(), value);
        auto function = m_modCtx.findFunction("__index__", args);
        if (!function)
            return m_logger.error(
                node->location,
                "No matching operator function __index__ for type "
                    + args.front()->toString(),
                nullptr
            );

        node->target = function;
        return (node->type = function->result);
    }
}

IRType* SemanticPass::visit(IRFieldExpression* node) {
    auto value = dispatch(node->expression);
    if (!value->isStructType())
        return m_logger.error(
            node->location,
            "Left side of field expression has to be of struct type",
            nullptr
        );

    auto structType = dynamic_cast<IRStructType*>(value);
    for (int i = 0; i < structType->fields.size(); i++) {
        if (structType->fields[i].first == node->fieldName)
            return (node->type = structType->fields[i].second);
    }

    return m_logger.error(
        node->location,
        "Struct " + structType->name + " does not have a field named "
            + node->fieldName,
        nullptr
    );
}

IRType* SemanticPass::visit(IRBlockStatement* node) {
    for (auto& statement : node->statements)
        dispatch(statement);

    return nullptr;
}

IRType* SemanticPass::visit(IRExpressionStatement* node) {
    dispatch(node->expression);
    return nullptr;
}

IRType* SemanticPass::visit(IRVariableStatement* node) {
    for (auto& [name, value] : node->items) {
        if (m_env->getVariableType(name))
            return m_logger.error(
                node->location, "Variable is already defined", nullptr
            );

        if (dispatch(value)->isVoidType())
            return m_logger.error(
                node->location, "Variable cannot have void type", nullptr
            );

        m_env->addVariableType(name, value->type);
    }

    return nullptr;
}

IRType* SemanticPass::visit(IRReturnStatement* node) {
    m_result = dispatch(node->expression);
    if (m_result != m_expectedResult) {
        m_result = nullptr;
        return m_logger.error(
            node->location,
            "Return expression has to be of function result type",
            nullptr
        );
    }

    return nullptr;
}

IRType* SemanticPass::visit(IRWhileStatement* node) {
    auto condition = dispatch(node->condition);
    if (!condition->isBoolType())
        return m_logger.error(
            node->location, "While condition has to be of boolean type", nullptr
        );

    auto prevResult = m_result;
    dispatch(node->body);
    m_result = prevResult;

    return nullptr;
}

IRType* SemanticPass::visit(IRIfStatement* node) {
    auto condition = dispatch(node->condition);
    if (!condition->isBoolType())
        return m_logger.error(
            node->location, "If condition has to be of boolean type", nullptr
        );

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

IRType* SemanticPass::visit(IRConstraintDeclaration* node) {
    return nullptr;
}

IRType* SemanticPass::visit(IRStructDeclaration* node) {
    return nullptr;
}

IRType* SemanticPass::visit(IRFunctionDeclaration* node) {
    if (!node->body)
        return nullptr;

    m_env = m_envCtx.make(Environment(node->name, m_env));
    setupEnvironment(node);

    for (auto& [paramName, paramType] : node->params)
        m_env->addVariableType(paramName, paramType);

    m_result = nullptr;
    m_expectedResult = node->result;

    dispatch(node->body);

    m_env = m_env->getParent();

    if (!m_expectedResult->isVoidType() && !m_result)
        return m_logger.error(
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

void SemanticPass::setupEnvironment(IRDeclaration* node) {
    for (auto typeParam : node->typeParams)
        m_env->addGeneric(typeParam);

    for (auto& [name, args] : node->requirements) {
        auto constraint = m_env->getConstraint(name);
        if (!constraint)
            return m_logger.error(
                node->location, "No constraint named " + name
            );

        if (args.size() != constraint->typeParams.size())
            return m_logger.error(
                node->location,
                "Number of args does not match number of type parameters for constraint "
                    + constraint->name
            );

        std::unordered_map<IRType*, IRType*> typeParamToArgLookup;
        for (size_t i = 0; i < args.size(); i++)
            typeParamToArgLookup.try_emplace(
                constraint->typeParams[i], args[i]
            );

        for (auto condition : constraint->conditions) {
            assert(condition && "Condition cannot be nullptr");
            auto function = dynamic_cast<IRFunctionDeclaration*>(condition);
            if (!function)
                return m_logger.error(
                    condition->location,
                    "Constraint condition has to be a function declaration"
                );

            auto params = function->params | std::views::transform([&](auto p) {
                              if (typeParamToArgLookup.contains(p.second))
                                  p.second = typeParamToArgLookup.at(p.second);
                              return p;
                          });

            m_env->addFunction(m_genericCtx.make(IRFunctionDeclaration(
                function->location,
                function->name,
                function->typeParams,
                std::vector<std::pair<std::string, std::vector<IRType*>>>(),
                function->result,
                std::vector(params.begin(), params.end()),
                nullptr
            )));
        }
    }
}