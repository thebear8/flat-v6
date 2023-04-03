#include "function_population_pass.hpp"

#include <cassert>
#include <ranges>

#include "../compiler.hpp"
#include "../environment.hpp"
#include "../util/error_logger.hpp"
#include "../util/graph_context.hpp"
#include "../util/string_switch.hpp"

IRModule* FunctionPopulationPass::process(ASTSourceFile* sourceFile)
{
    return (IRModule*)dispatch(sourceFile);
}

IRNode* FunctionPopulationPass::visit(ASTIntegerExpression* node)
{
    auto radix = StringSwitch<std::size_t>(node->value)
                     .StartsWith("0x", 16)
                     .StartsWith("0b", 2)
                     .Default(10);

    auto value = StringSwitch<std::string>(node->value)
                     .StartsWith(
                         "0x",
                         node->value.substr(
                             std::min<std::size_t>(2, node->value.length())
                         )
                     )
                     .StartsWith(
                         "0b",
                         node->value.substr(
                             std::min<std::size_t>(2, node->value.length())
                         )
                     )
                     .Default(node->value);

    auto width = StringSwitch<std::size_t>(node->suffix)
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

IRNode* FunctionPopulationPass::visit(ASTBoolExpression* node)
{
    auto value = StringSwitch<bool>(node->value)
                     .Case("true", true)
                     .Case("false", false)
                     .OrThrow();

    return m_irCtx->make(IRBoolExpression(value))->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTCharExpression* node)
{
    std::size_t position = 0;
    return m_irCtx
        ->make(IRCharExpression(
            unescapeCodePoint(node->value, position, node->location)
        ))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTStringExpression* node)
{
    return m_irCtx
        ->make(
            IRStringExpression(unescapeStringUTF8(node->value, node->location))
        )
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTIdentifierExpression* node)
{
    std::vector<IRType*> typeArgs;
    for (auto typeArg : node->typeArgs)
    {
        auto&& [irType, error] = m_resolver.resolve(typeArg, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(node->location, error, nullptr);
        typeArgs.push_back(irType);
    }

    return m_irCtx->make(IRIdentifierExpression(node->value, typeArgs))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTStructExpression* node)
{
    std::vector<IRType*> typeArgs;
    for (auto typeArg : node->typeArgs)
    {
        auto&& [irType, error] = m_resolver.resolve(typeArg, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(node->location, error, nullptr);
        typeArgs.push_back(irType);
    }

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

        fields.try_emplace(name, (IRExpression*)dispatch(value));
    }

    return m_irCtx->make(IRStructExpression(node->structName, typeArgs, fields))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTUnaryExpression* node)
{
    return m_irCtx
        ->make(IRUnaryExpression(
            node->operation, (IRExpression*)dispatch(node->expression)
        ))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTBinaryExpression* node)
{
    return m_irCtx
        ->make(IRBinaryExpression(
            node->operation,
            (IRExpression*)dispatch(node->left),
            (IRExpression*)dispatch(node->right)
        ))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTCallExpression* node)
{
    std::vector<IRExpression*> args;
    for (auto arg : node->args)
        args.push_back((IRExpression*)dispatch(arg));

    return m_irCtx
        ->make(IRCallExpression((IRExpression*)dispatch(node->expression), args)
        )
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTIndexExpression* node)
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

IRNode* FunctionPopulationPass::visit(ASTFieldExpression* node)
{
    return m_irCtx
        ->make(IRFieldExpression(
            (IRExpression*)dispatch(node->expression), node->fieldName
        ))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTBlockStatement* node)
{
    std::vector<IRStatement*> statements;
    for (auto statement : node->statements)
        statements.push_back((IRStatement*)dispatch(statement));

    return m_irCtx->make(IRBlockStatement(statements))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTExpressionStatement* node)
{
    return m_irCtx
        ->make(IRExpressionStatement((IRExpression*)dispatch(node->expression)))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTVariableStatement* node)
{
    std::vector<std::pair<std::string, IRExpression*>> items;
    for (auto const& [name, value] : node->items)
        items.push_back({ name, (IRExpression*)dispatch(value) });

    return m_irCtx->make(IRVariableStatement(items))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTReturnStatement* node)
{
    return m_irCtx
        ->make(IRReturnStatement((IRExpression*)dispatch(node->expression)))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTWhileStatement* node)
{
    return m_irCtx
        ->make(IRWhileStatement(
            (IRExpression*)dispatch(node->condition),
            (IRStatement*)dispatch(node->body)
        ))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTIfStatement* node)
{
    return m_irCtx
        ->make(IRIfStatement(
            (IRExpression*)dispatch(node->condition),
            (IRStatement*)dispatch(node->ifBody),
            (IRStatement*)((node->elseBody) ? dispatch(node->elseBody) : nullptr)
        ))
        ->setLocation(node->location);
}

IRNode* FunctionPopulationPass::visit(ASTRequirement* node)
{
    auto constraint = m_env->findConstraint(node->constraintName);

    if (!constraint)
    {
        return m_logger.error(
            node->location,
            "No constraint named " + node->constraintName,
            nullptr
        );
    }

    if (node->typeArgs.size() != constraint->typeParams.size())
    {
        return m_logger.error(
            node->location,
            "Number of type args does not match number of type parameters",
            nullptr
        );
    }

    std::vector<IRType*> typeArgs;
    for (auto typeArg : node->typeArgs)
    {
        auto&& [irType, error] = m_resolver.resolve(typeArg, m_env, m_irCtx);
        if (!irType)
            return m_logger.error(typeArg->location, error, nullptr);

        typeArgs.push_back(irType);
    }

    auto instantiation =
        m_env->findConstraintInstantiation(constraint, typeArgs);

    if (!instantiation)
    {
        instantiation = m_env->addConstraintInstantiation(
            constraint,
            m_instantiator.makeConstraintInstantiation(constraint, typeArgs)
        );
    }

    return instantiation;
}

IRNode* FunctionPopulationPass::visit(ASTFunctionDeclaration* node)
{
    m_env = m_irCtx->make(Environment(node->name, m_module->getEnv()));

    auto function = node->getIRFunction();
    for (auto typeParam : function->typeParams)
        m_env->addTypeParam(typeParam);

    for (auto requirement : node->requirements)
    {
        auto instantiation = (IRConstraintInstantiation*)dispatch(requirement);

        if (function->requirements.contains(instantiation))
        {
            return m_logger.error(
                requirement->location,
                "Requirement of same type already exists",
                nullptr
            );
        }

        function->requirements.emplace(instantiation);
    }

    function->body = node->body ? (IRStatement*)dispatch(node->body) : nullptr;

    m_env = nullptr;
    return function;
}

IRNode* FunctionPopulationPass::visit(ASTSourceFile* node)
{
    m_module = node->getIRModule();
    m_irCtx = m_module->getIrCtx();

    for (auto declaration : node->declarations)
        dispatch(declaration);

    return m_module;
}

std::vector<uint8_t> FunctionPopulationPass::unescapeStringUTF8(
    std::string const& input, SourceRef const& location
)
{
    std::vector<uint8_t> bytes;

    std::size_t position = 0;
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

uint32_t FunctionPopulationPass::unescapeCodePoint(
    std::string const& input, std::size_t& position, SourceRef const& location
)
{
    if (position < input.length() && input[position] == '\\')
    {
        position++;
        if (position < input.length()
            && isDigit(input[position]))  // octal char
                                          // literal
        {
            std::size_t start = position;
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
            std::size_t start = position;
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
            std::size_t start = position;
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
            std::size_t start = position;
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
            if (!m_escapeChars.contains(input[position]))
                return m_logger.error(location, "Invalid escape sequence", 0);

            position++;
            return m_escapeChars.at(input[position]);
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