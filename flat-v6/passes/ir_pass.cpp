#include "ir_pass.hpp"

#include <cassert>
#include <ranges>

#include "../util/string_switch.hpp"

IRModule* IRPass::process(ASTSourceFile* sourceFile)
{
    return (IRModule*)dispatch(sourceFile);
}

IRNode* IRPass::visit(ASTIntegerExpression* node)
{
    auto radix = StringSwitch<size_t>(node->value)
                     .StartsWith("0x", 16)
                     .StartsWith("0b", 2)
                     .Default(10);

    auto value =
        StringSwitch<std::string>(node->value)
            .StartsWith(
                "0x",
                node->value.substr(std::min<size_t>(2, node->value.length()))
            )
            .StartsWith(
                "0b",
                node->value.substr(std::min<size_t>(2, node->value.length()))
            )
            .Default(node->value);

    auto width = StringSwitch<size_t>(node->suffix)
                     .EndsWith("8", 8)
                     .EndsWith("16", 16)
                     .EndsWith("32", 32)
                     .EndsWith("64", 64)
                     .Case("", 32)
                     .OrThrow();

    auto isSigned = StringSwitch<bool>(node->suffix)
                        .StartsWith("u", false)
                        .StartsWith("i", true)
                        .Case("", true)
                        .OrThrow();

    return m_irCtx->make(IRIntegerExpression(isSigned, width, radix, value))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTBoolExpression* node)
{
    auto value = StringSwitch<bool>(node->value)
                     .Case("true", true)
                     .Case("false", false)
                     .OrThrow();

    return m_irCtx->make(IRBoolExpression(value))->setLocation(node->location);
}

IRNode* IRPass::visit(ASTCharExpression* node)
{
    size_t position = 0;
    return m_irCtx
        ->make(IRCharExpression(
            unescapeCodePoint(node->value, position, node->location)
        ))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTStringExpression* node)
{
    return m_irCtx
        ->make(
            IRStringExpression(unescapeStringUTF8(node->value, node->location))
        )
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTIdentifierExpression* node)
{
    std::vector<IRType*> typeArgs;
    for (auto typeArg : node->typeArgs)
        typeArgs.push_back((IRType*)dispatch(typeArg));

    return m_irCtx->make(IRIdentifierExpression(node->value, typeArgs))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTStructExpression* node)
{
    std::vector<IRType*> typeArgs;
    for (auto typeArg : node->typeArgs)
        typeArgs.push_back((IRType*)dispatch(typeArg));

    std::unordered_map<std::string, IRExpression*> fields;
    for (auto const& [name, value] : node->fields)
    {
        if (fields.contains(name))
        {
            return m_logger.error(
                node->location,
                "Multiple initializers for field " + name + " of struct "
                    + node->structName,
                nullptr
            );
        }

        fields.try_emplace(name, value);
    }

    return m_irCtx->make(IRStructExpression(node->structName, typeArgs, fields))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTUnaryExpression* node)
{
    return m_irCtx
        ->make(IRUnaryExpression(
            node->operation, (IRExpression*)dispatch(node->expression)
        ))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTBinaryExpression* node)
{
    return m_irCtx
        ->make(IRBinaryExpression(
            node->operation,
            (IRExpression*)dispatch(node->left),
            (IRExpression*)dispatch(node->right)
        ))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTCallExpression* node)
{
    std::vector<IRExpression*> args;
    for (auto arg : node->args)
        args.push_back((IRExpression*)dispatch(arg));

    return m_irCtx
        ->make(IRCallExpression((IRExpression*)dispatch(node->expression), args)
        )
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTIndexExpression* node)
{
    std::vector<IRExpression*> args;
    for (auto arg : node->args)
        args.push_back((IRExpression*)dispatch(arg));

    return m_irCtx
        ->make(
            IRIndexExpression((IRExpression*)dispatch(node->expression), args)
        )
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTFieldExpression* node)
{
    return m_irCtx
        ->make(IRFieldExpression(
            (IRExpression*)dispatch(node->expression), node->fieldName
        ))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTBlockStatement* node)
{
    std::vector<IRStatement*> statements;
    for (auto statement : node->statements)
        statements.push_back((IRStatement*)dispatch(statement));

    return m_irCtx->make(IRBlockStatement(statements))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTExpressionStatement* node)
{
    return m_irCtx
        ->make(IRExpressionStatement((IRExpression*)dispatch(node->expression)))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTVariableStatement* node)
{
    std::vector<std::pair<std::string, IRExpression*>> items;
    for (auto const& [name, value] : node->items)
        items.push_back({ name, (IRExpression*)dispatch(value) });

    return m_irCtx->make(IRVariableStatement(items))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTReturnStatement* node)
{
    return m_irCtx
        ->make(IRReturnStatement((IRExpression*)dispatch(node->expression)))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTWhileStatement* node)
{
    return m_irCtx
        ->make(IRWhileStatement(
            (IRExpression*)dispatch(node->condition),
            (IRStatement*)dispatch(node->body)
        ))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTIfStatement* node)
{
    return m_irCtx
        ->make(IRIfStatement(
            (IRExpression*)dispatch(node->condition),
            (IRStatement*)dispatch(node->ifBody),
            (IRStatement*)((node->elseBody) ? dispatch(node->elseBody) : nullptr)
        ))
        ->setLocation(node->location);
}

IRNode* IRPass::visit(ASTConstraintDeclaration* node)
{
    m_env = m_irCtx->make(Environment(node->name, m_env));

    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    for (auto p : typeParams)
        m_env->addTypeParam(p);

    std::vector<IRFunction*> conditions;
    for (auto condition : node->conditions)
        conditions.push_back((IRFunction*)dispatch(condition));

    auto irNode = m_irCtx->make(IRConstraint(
        node->name,
        typeParams,
        transformRequirements(node->requirements),
        std::vector(conditions.begin(), conditions.end())
    ));

    irNode->setLocation(node->location);
    m_env = m_env->getParent();
    if (!m_env->addConstraint(irNode))
    {
        return m_logger.error(
            node->location,
            "Constraint " + node->name + " in module " + m_module->name
                + " is already defined",
            nullptr
        );
    }

    m_module->constraints.push_back(irNode);
    return irNode;
}

IRNode* IRPass::visit(ASTStructDeclaration* node)
{
    m_env = m_irCtx->make(Environment(node->name, m_env));

    auto structType = node->getIRStructType();

    for (auto typeParam : structType->typeParams)
        m_env->addTypeParam(typeParam);

    for (auto const& [name, type] : node->fields)
    {
        if (structType->fields.contains(name))
        {
            return m_logger.error(
                node->location,
                "Field " + name + " of struct " + node->name
                    + " has multiple definitions",
                nullptr
            );
        }

        structType->fields.try_emplace(name, (IRType*)dispatch(type));
    }

    m_env = m_env->getParent();
    return structType;
}

IRNode* IRPass::visit(ASTFunctionDeclaration* node)
{
    m_env = m_irCtx->make(Environment(node->name, m_env));

    std::vector<IRGenericType*> typeParams;
    for (auto typeParam : node->typeParams)
        typeParams.push_back(m_irCtx->make(IRGenericType(typeParam)));

    for (auto p : typeParams)
        m_env->addTypeParam(p);

    std::vector<std::pair<std::string, IRType*>> params;
    for (auto const& [name, type] : node->parameters)
        params.push_back({ name, (IRType*)dispatch(type) });

    IRFunction* function = nullptr;
    if (node->body)
    {
        function = m_irCtx->make(IRFunction(
            node->name,
            typeParams,
            transformRequirements(node->requirements),
            params,
            (IRType*)dispatch(node->result),
            (IRStatement*)dispatch(node->body)
        ));
    }
    else
    {
        function = m_irCtx->make(IRFunction(
            node->lib,
            node->name,
            typeParams,
            transformRequirements(node->requirements),
            params,
            (IRType*)dispatch(node->result)
        ));
    }

    function->setLocation(node->location);
    m_env = m_env->getParent();
    if (!m_env->addFunction(function))
    {
        return m_logger.error(
            node->location,
            "Function " + node->name + " in module " + m_module->name
                + " is already defined",
            nullptr
        );
    }

    m_module->functions.push_back(function);
    return function;
}

IRNode* IRPass::visit(ASTSourceFile* node)
{
    std::string name;
    for (auto const& segment : node->modulePath)
        name += ((name.empty()) ? "" : ".") + segment;

    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();
    m_env = m_module->getEnv();

    for (auto declaration : node->declarations)
        dispatch(declaration);

    return m_module;
}

IRNode* IRPass::visit(ASTNamedType* node)
{
    if (auto builtinType = m_env->findBuiltinType(node->name))
    {
        return builtinType;
    }
    else if (auto typeParam = m_env->findTypeParam(node->name))
    {
        return typeParam;
    }
    if (auto structType = m_env->findStruct(node->name);
        structType || node->typeArgs.size() != 0)
    {
        if (!structType)
        {
            return m_logger.error(
                node->location, "No struct named " + node->name, nullptr
            );
        }

        if (structType->typeParams.size() != node->typeArgs.size())
        {
            return m_logger.error(
                node->location,
                "Number of type arguments for struct " + node->name
                    + " does not match number of type parameters",
                nullptr
            );
        }

        std::vector<IRType*> typeArgs;
        for (auto typeArg : node->typeArgs)
            typeArgs.push_back((IRType*)dispatch(typeArg));

        auto& structInstantiations = m_module->getStructInstantiations();

        if (!structInstantiations.contains(structType))
        {
            structInstantiations.try_emplace(
                structType,
                std::map<std::vector<IRType*>, IRStructInstantiation*>()
            );
        }

        if (!structInstantiations.at(structType).contains(typeArgs))
        {
            auto instantiation =
                m_irCtx->make(IRStructInstantiation(structType, typeArgs));

            structInstantiations.at(structType)
                .try_emplace(typeArgs, instantiation);
        }

        return structInstantiations.at(structType).at(typeArgs);
    }
    else
    {
        return m_logger.error(
            node->location, "No type named " + node->name, nullptr
        );
    }
}

IRNode* IRPass::visit(ASTPointerType* node)
{
    return m_compCtx.getPointerType((IRType*)dispatch(node->base));
}

IRNode* IRPass::visit(ASTArrayType* node)
{
    return m_compCtx.getArrayType((IRType*)dispatch(node->base));
}

std::vector<std::pair<std::string, std::vector<IRType*>>>
IRPass::transformRequirements(
    std::vector<std::pair<std::string, std::vector<ASTType*>>> const&
        requirements
)
{
    auto t = [&](std::pair<std::string, std::vector<ASTType*>> const& c) {
        auto t = [&](ASTType* t) {
            return (IRType*)dispatch(t);
        };

        auto const& [name, typeArgs] = c;
        auto range = typeArgs | std::views::transform(t);
        return std::pair(name, std::vector(range.begin(), range.end()));
    };

    auto range = requirements | std::views::transform(t);
    return std::vector(range.begin(), range.end());
}

std::vector<uint8_t> IRPass::unescapeStringUTF8(
    std::string const& input, SourceRef const& location
)
{
    std::vector<uint8_t> bytes;

    size_t position = 0;
    while (position < input.length())
    {
        uint32_t cp = unescapeCodePoint(input, position, location);
        if (cp < 0x7F)
        {
            bytes.push_back((uint8_t)cp);
        }
        else if (cp <= 0x07FF)
        {
            bytes.push_back(((cp >> 6) & 0x1F) | 0xC0);
            bytes.push_back(((cp >> 0) & 0x3F) | 0x80);
        }
        else if (cp <= 0xFFFF)
        {
            bytes.push_back(((cp >> 12) & 0x0F) | 0xE0);
            bytes.push_back(((cp >> 6) & 0x3F) | 0x80);
            bytes.push_back(((cp >> 0) & 0x3F) | 0x80);
        }
        else if (cp <= 0x10FFFF)
        {
            bytes.push_back(((cp >> 18) & 0x07) | 0xF0);
            bytes.push_back(((cp >> 12) & 0x3F) | 0x80);
            bytes.push_back(((cp >> 6) & 0x3F) | 0x80);
            bytes.push_back(((cp >> 0) & 0x3F) | 0x80);
        }
        else
        {
            return m_logger.error(
                location, "Invalid Unicode code point", bytes
            );
        }
    }

    bytes.push_back(0);
    return bytes;
}

uint32_t IRPass::unescapeCodePoint(
    std::string const& input, size_t& position, SourceRef const& location
)
{
    if (position < input.length() && input[position] == '\\')
    {
        position++;
        if (position < input.length()
            && isDigit(input[position]))  // octal char
                                          // literal
        {
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) > 3)
                return m_logger.error(
                    location,
                    "Octal char literal cannot have more than three digits",
                    0
                );

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 8
            );
        }
        else if (position < input.length() && input[position] == 'x')  // hex
                                                                       // char
                                                                       // literal
        {
            position++;
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) == 0)
                return m_logger.error(
                    location, "Hex char literal cannot have zero digits", 0
                );

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 16
            );
        }
        else if (position < input.length() && input[position] == 'u')  // 0xhhhh
                                                                       // unicode
                                                                       // code
                                                                       // point
        {
            position++;
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) != 4)
                m_logger.error(
                    location,
                    "2 byte Unicode code point (\\u) must have 4 digits",
                    ""
                );

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 16
            );
        }
        else if (position < input.length() && input[position] == 'U')  // 0xhhhhhhhh
                                                                       // unicode
                                                                       // code
                                                                       // point
        {
            position++;
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) != 8)
                m_logger.error(
                    location,
                    "4 byte Unicode code point (\\U) must have 8 digits",
                    ""
                );

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 16
            );
        }
        else if (position < input.length())
        {
            if (!escapeChars.contains(input[position]))
                return m_logger.error(location, "Invalid escape sequence", 0);

            position++;
            return escapeChars.at(input[position]);
        }
        else
        {
            return m_logger.error(location, "Incomplete escape sequence", 0);
        }
    }
    else if (position < input.length())
    {
        return input[position++];
    }
    else
    {
        return m_logger.error(location, "Unexpected end of char sequence", 0);
    }
}