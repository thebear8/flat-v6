#include "semantic_pass.hpp"

void SemanticPass::analyze(IRSourceFile* program)
{
    dispatch(program);
}

Type* SemanticPass::visit(IRIntegerExpression* node)
{
    return (node->type = compCtx.getIntegerType(node->width, node->isSigned));
}

Type* SemanticPass::visit(IRBoolExpression* node)
{
    return (node->type = compCtx.getBool());
}

Type* SemanticPass::visit(IRCharExpression* node)
{
    return (node->type = compCtx.getChar());
}

Type* SemanticPass::visit(IRStringExpression* node)
{
    return (node->type = compCtx.getString());
}

Type* SemanticPass::visit(IRIdentifierExpression* node)
{
    if (!localVariables.contains(node->value))
        return logger.error(node->location, "Undefined Identifier", nullptr);

    return (node->type = localVariables.at(node->value));
}

Type* SemanticPass::visit(IRStructExpression* node)
{
    auto structType = modCtx.resolveStruct(node->structName);
    if (!structType)
        return logger.error(
            node->location, 
            "Undefined Struct Type " + node->structName, nullptr);

    for (auto& [name, value] : node->fields)
        dispatch(value);

    for (auto& [name, value] : node->fields)
    {
        for (int i = 0; i < structType->fields.size(); i++)
        {
            auto& [fieldName, fieldType] = structType->fields[i];
            if (fieldName == name)
            {
                if (fieldType != value->type)
                    return logger.error(
                        node->location, 
                        "Field " + name + " has type " + fieldType->toString()
                            + ", value type is " + value->type->toString(),
                        nullptr);
                break;
            }

            if (i == structType->fields.size() - 1)
                return logger.error(
                    node->location, 
                    "Struct " + structType->name
                        + " does not contain a field called " + name,
                    nullptr);
        }
    }

    for (auto& [fieldName, fieldType] : structType->fields)
    {
        for (int i = 0; i < node->fields.size(); i++)
        {
            auto& [name, value] = node->fields[i];
            if (name == fieldName)
            {
                if (value->type != fieldType)
                    return logger.error(
                        node->location, 
                        "Field " + name + " has type " + fieldType->toString()
                            + ", value type is " + value->type->toString(),
                        nullptr);
                break;
            }

            if (i == node->fields.size() - 1)
                return logger.error(
                    node->location, 
                    "No initializer for field " + fieldName + ": "
                        + fieldType->toString(),
                    nullptr);
        }
    }

    return (node->type = structType);
}

Type* SemanticPass::visit(IRUnaryExpression* node)
{
    auto value = dispatch(node->expression);
    if (unaryOperators.at(node->operation).category
            == OperatorCategory::UnaryArithmetic
        && value->isIntegerType())
    {
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
        return (node->type = compCtx.getBool());
    }
    else
    {
        auto args = std::vector({ value });
        auto function = modCtx.resolveFunction(
            unaryOperators.at(node->operation).name, args);
        if (!function)
            return logger.error(
                node->location, 
                "No matching operator function "
                    + unaryOperators.at(node->operation).name + " for type "
                    + value->toString(),
                nullptr);

        node->target = function;
        return (node->type = function->result);
    }
}

Type* SemanticPass::visit(IRBinaryExpression* node)
{
    auto left = dispatch(node->left);
    auto right = dispatch(node->right);

    if (binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryArithmetic
        && (left->isIntegerType() && right->isIntegerType()))
    {
        return (
            node->type =
                ((left->getBitSize() >= right->getBitSize()) ? left : right));
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
        return (node->type = compCtx.getBool());
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryLogic
        && (left->isBoolType() && right->isBoolType()))
    {
        return (node->type = compCtx.getBool());
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryEquality
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        return (node->type = compCtx.getBool());
    }
    else if (
        binaryOperators.at(node->operation).category
            == OperatorCategory::BinaryAssign
        && ((left == right)
            || (left->isIntegerType() && right->isIntegerType())))
    {
        if (!dynamic_cast<IRIdentifierExpression*>(node->left))
            return logger.error(
                node->location, 
                "Left side of assignment has to be identifier", nullptr);

        if ((left->isIntegerType() && right->isIntegerType())
            && (left->getBitSize() < right->getBitSize()))
            logger.warning(
                node->location, 
                "Narrowing conversion from " + left->toString() + " to "
                + right->toString());

        return (node->type = left);
    }
    else
    {
        auto args = std::vector({ left, right });
        auto function = modCtx.resolveFunction(
            binaryOperators.at(node->operation).name, args);
        if (!function)
            return logger.error(
                node->location, 
                "No matching operator function "
                    + binaryOperators.at(node->operation).name + " for types "
                    + left->toString() + ", " + right->toString(),
                nullptr);

        if (binaryOperators.at(node->operation).category
                == OperatorCategory::BinaryAssign
            && function->result != left)
            return logger.error(
                node->location, 
                "Assignment operator overload function has to return a value that has the type of the left operand",
                nullptr);

        node->target = function;
        return (node->type = function->result);
    }
}

Type* SemanticPass::visit(IRCallExpression* node)
{
    std::vector<Type*> args;
    for (auto arg : node->args)
        args.push_back(dispatch(arg));

    if (auto identifierExpression =
            dynamic_cast<IRIdentifierExpression*>(node->expression))
    {
        auto function =
            modCtx.resolveFunction(identifierExpression->value, args);
        if (!function)
            return logger.error(
                node->location, 
                "No matching function " + identifierExpression->value, nullptr);

        node->target = function;
        return (node->type = function->result);
    }
    else
    {
        args.insert(args.begin(), dispatch(node->expression));
        auto function = modCtx.resolveFunction("__call__", args);
        if (!function)
            return logger.error(
                node->location, 
                "No matching operator function __call__ for type "
                    + args.front()->toString(),
                nullptr);

        node->target = function;
        return (node->type = function->result);
    }
}

Type* SemanticPass::visit(IRIndexExpression* node)
{
    std::vector<Type*> args;
    for (auto arg : node->args)
        args.push_back(dispatch(arg));

    auto value = dispatch(node->expression);
    if (value->isArrayType() && args.size() == 1
        && args.front()->isIntegerType())
    {
        return (node->type = dynamic_cast<ArrayType*>(value)->base);
    }
    if (value->isStringType() && args.size() == 1
        && args.front()->isIntegerType())
    {
        return (node->type = compCtx.getU8());
    }
    else
    {
        args.insert(args.begin(), value);
        auto function = modCtx.resolveFunction("__index__", args);
        if (!function)
            return logger.error(
                node->location, 
                "No matching operator function __index__ for type "
                    + args.front()->toString(),
                nullptr);

        node->target = function;
        return (node->type = function->result);
    }
}

Type* SemanticPass::visit(IRFieldExpression* node)
{
    auto value = dispatch(node->expression);
    if (!value->isStructType())
        return logger.error(
            node->location, 
            "Left side of field expression has to be of struct type", nullptr);

    auto structType = dynamic_cast<StructType*>(value);
    for (int i = 0; i < structType->fields.size(); i++)
    {
        if (structType->fields[i].first == node->fieldName)
            return (node->type = structType->fields[i].second);
    }

    return logger.error(
        node->location, 
        "Struct " + structType->name + " does not have a field named "
            + node->fieldName,
        nullptr);
}

Type* SemanticPass::visit(IRBlockStatement* node)
{
    for (auto& statement : node->statements)
        dispatch(statement);

    return nullptr;
}

Type* SemanticPass::visit(IRExpressionStatement* node)
{
    dispatch(node->expression);
    return nullptr;
}

Type* SemanticPass::visit(IRVariableStatement* node)
{
    for (auto& [name, value] : node->items)
    {
        if (localVariables.contains(name))
            return logger.error(
                node->location, "Variable is already defined", nullptr);

        if (dispatch(value)->isVoidType())
            return logger.error(
                node->location, "Variable cannot have void type", nullptr);

        localVariables.try_emplace(name, value->type);
    }

    return nullptr;
}

Type* SemanticPass::visit(IRReturnStatement* node)
{
    functionResult = dispatch(node->expression);
    if (functionResult != expectedFunctionResult)
    {
        functionResult = nullptr;
        return logger.error(
            node->location, 
            "Return expression has to be of function result type", nullptr);
    }

    return nullptr;
}

Type* SemanticPass::visit(IRWhileStatement* node)
{
    auto condition = dispatch(node->condition);
    if (!condition->isBoolType())
        return logger.error(
            node->location, 
            "While condition has to be of boolean type", nullptr);

    auto prevResult = functionResult;
    dispatch(node->body);
    functionResult = prevResult;

    return nullptr;
}

Type* SemanticPass::visit(IRIfStatement* node)
{
    auto condition = dispatch(node->condition);
    if (!condition->isBoolType())
        return logger.error(
            node->location, "If condition has to be of boolean type", nullptr);

    auto prevResult = functionResult;

    functionResult = nullptr;
    dispatch(node->ifBody);
    auto ifResult = functionResult;

    functionResult = nullptr;
    if (node->elseBody)
        dispatch(node->elseBody);
    auto elseResult = functionResult;

    functionResult =
        (((ifResult != nullptr) && (elseResult != nullptr)) ? ifResult
                                                            : prevResult);

    return nullptr;
}

Type* SemanticPass::visit(IRStructDefinition* node)
{
    return nullptr;
}

Type* SemanticPass::visit(IRFunctionDefinition* node)
{
    if (!node->body)
        return nullptr;

    localVariables.clear();
    for (auto& param : node->params)
        localVariables.try_emplace(param.first, param.second);

    functionResult = nullptr;
    expectedFunctionResult = node->result;

    dispatch(node->body);

    if (!expectedFunctionResult->isVoidType() && !functionResult)
        return logger.error(
            node->location, 
            "Missing return statement in function " + node->name
                + ", should return " + expectedFunctionResult->toString(),
            nullptr);

    return nullptr;
}

Type* SemanticPass::visit(IRSourceFile* node)
{
    for (auto& decl : node->definitions)
        dispatch(decl);

    return nullptr;
}