#include "ir_pass.hpp"

#include "../util/string_switch.hpp"

IRSourceFile* IRPass::process(ASTSourceFile* sourceFile)
{
    return (IRSourceFile*)dispatch(sourceFile);
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
                node->value.substr(std::min<size_t>(2, node->value.length())))
            .StartsWith(
                "0b",
                node->value.substr(std::min<size_t>(2, node->value.length())))
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

    return irCtx.make(IRIntegerExpression(node->location, isSigned, width, radix, value));
}

IRNode* IRPass::visit(ASTBoolExpression* node)
{
    auto value = StringSwitch<bool>(node->value)
                     .Case("true", true)
                     .Case("false", false)
                     .OrThrow();

    return irCtx.make(IRBoolExpression(node->location, value));
}

IRNode* IRPass::visit(ASTCharExpression* node)
{
    size_t position = 0;
    return irCtx.make(IRCharExpression(
        node->location, 
        unescapeCodePoint(node->value, position, node->location)));
}

IRNode* IRPass::visit(ASTStringExpression* node)
{
    return irCtx.make(IRStringExpression(
        node->location, unescapeStringUTF8(node->value, node->location)));
}

IRNode* IRPass::visit(ASTIdentifierExpression* node)
{
    return irCtx.make(IRIdentifierExpression(node->location, node->value));
}

IRNode* IRPass::visit(ASTStructExpression* node)
{
    std::vector<std::pair<std::string, IRExpression*>> fields;
    for (auto const& [name, value] : node->fields)
        fields.push_back({ name, (IRExpression*)dispatch(value) });

    return irCtx.make(
        IRStructExpression(node->location, node->structName, fields));
}

IRNode* IRPass::visit(ASTUnaryExpression* node)
{
    return irCtx.make(IRUnaryExpression(
        node->location, 
        node->operation, (IRExpression*)dispatch(node->expression), nullptr));
}

IRNode* IRPass::visit(ASTBinaryExpression* node)
{
    return irCtx.make(IRBinaryExpression(
        node->location, 
        node->operation,
        (IRExpression*)dispatch(node->left),
        (IRExpression*)dispatch(node->right),
        nullptr));
}

IRNode* IRPass::visit(ASTCallExpression* node)
{
    std::vector<IRExpression*> args;
    for (auto arg : node->args)
        args.push_back((IRExpression*)dispatch(arg));

    return irCtx.make(IRCallExpression(
        node->location, 
        (IRExpression*)dispatch(node->expression), args, nullptr));
}

IRNode* IRPass::visit(ASTIndexExpression* node)
{
    std::vector<IRExpression*> args;
    for (auto arg : node->args)
        args.push_back((IRExpression*)dispatch(arg));

    return irCtx.make(IRIndexExpression(
        node->location, 
        (IRExpression*)dispatch(node->expression), args, nullptr));
}

IRNode* IRPass::visit(ASTFieldExpression* node)
{
    return irCtx.make(IRFieldExpression(
        node->location, 
        (IRExpression*)dispatch(node->expression), node->fieldName));
}

IRNode* IRPass::visit(ASTBlockStatement* node)
{
    std::vector<IRStatement*> statements;
    for (auto statement : node->statements)
        statements.push_back((IRStatement*)dispatch(statement));

    return irCtx.make(IRBlockStatement(node->location, statements));
}

IRNode* IRPass::visit(ASTExpressionStatement* node)
{
    return irCtx.make(IRExpressionStatement(
        node->location, (IRExpression*)dispatch(node->expression)));
}

IRNode* IRPass::visit(ASTVariableStatement* node)
{
    std::vector<std::pair<std::string, IRExpression*>> items;
    for (auto const& [name, value] : node->items)
        items.push_back({ name, (IRExpression*)dispatch(value) });

    return irCtx.make(IRVariableStatement(node->location, items));
}

IRNode* IRPass::visit(ASTReturnStatement* node)
{
    return irCtx.make(IRReturnStatement(
        node->location, (IRExpression*)dispatch(node->expression)));
}

IRNode* IRPass::visit(ASTWhileStatement* node)
{
    return irCtx.make(IRWhileStatement(
        node->location, 
        (IRExpression*)dispatch(node->condition),
        (IRStatement*)dispatch(node->body)));
}

IRNode* IRPass::visit(ASTIfStatement* node)
{
    return irCtx.make(IRIfStatement(
        node->location, 
        (IRExpression*)dispatch(node->condition),
        (IRStatement*)dispatch(node->ifBody),
        (IRStatement*)((node->elseBody) ? dispatch(node->elseBody) : nullptr)));
}

IRNode* IRPass::visit(ASTStructDeclaration* node)
{
    std::vector<std::pair<std::string, Type*>> fields;
    for (auto const& [name, type] : node->fields)
        fields.push_back({ name, modCtx.getType(type) });

    return irCtx.make(IRStructDeclaration(node->location, node->name, fields));
}

IRNode* IRPass::visit(ASTFunctionDeclaration* node)
{
    std::vector<std::pair<std::string, Type*>> params;
    for (auto const& [name, type] : node->parameters)
        params.push_back({ name, modCtx.getType(type) });

    return irCtx.make(IRFunctionDeclaration(
        node->location, 
        node->name,
        modCtx.getType(node->result),
        params,
        (IRStatement*)dispatch(node->body)));
}

IRNode* IRPass::visit(ASTExternFunctionDeclaration* node)
{
    std::vector<std::pair<std::string, Type*>> params;
    for (auto const& [name, type] : node->parameters)
        params.push_back({ name, modCtx.getType(type) });

    return irCtx.make(IRFunctionDeclaration(
        node->location, 
        node->lib, node->name, modCtx.getType(node->result), params));
}

IRNode* IRPass::visit(ASTSourceFile* node)
{
    std::vector<IRDeclaration*> declarations;
    for (auto declaration : node->declarations)
        declarations.push_back((IRDeclaration*)dispatch(declaration));

    return irCtx.make(IRSourceFile(
        node->location, node->modulePath, node->importPaths, declarations));
}

std::vector<uint8_t> IRPass::unescapeStringUTF8(
    std::string const& input, SourceRef const& location)
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
            return logger.error(location, "Invalid Unicode code point", bytes);
        }
    }

    bytes.push_back(0);
    return bytes;
}

uint32_t IRPass::unescapeCodePoint(
    std::string const& input, size_t& position, SourceRef const& location)
{
    if (position < input.length() && input[position] == '\\')
    {
        position++;
        if (position < input.length()
            && isDigit(input[position]))  // octal char literal
        {
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) > 3)
                return logger.error(
                    location,
                    "Octal char literal cannot have more than three digits",
                    0);

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 8);
        }
        else if (
            position < input.length()
            && input[position] == 'x')  // hex char literal
        {
            position++;
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) == 0)
                return logger.error(
                    location, "Hex char literal cannot have zero digits", 0);

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 16);
        }
        else if (
            position < input.length() && input[position] == 'u')  // 0xhhhh
                                                                  // unicode
                                                                  // code
                                                                  // point
        {
            position++;
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) != 4)
                logger.error(
                    location,
                    "2 byte Unicode code point (\\u) must have 4 digits",
                    "");

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 16);
        }
        else if (
            position < input.length() && input[position] == 'U')  // 0xhhhhhhhh
                                                                  // unicode
                                                                  // code
                                                                  // point
        {
            position++;
            size_t start = position;
            while (position < input.length() && isDigit(input[position]))
                position++;

            if ((position - start) != 8)
                logger.error(
                    location,
                    "4 byte Unicode code point (\\U) must have 8 digits",
                    "");

            return std::stoul(
                input.substr(start, (position - start)), nullptr, 16);
        }
        else if (position < input.length())
        {
            if (!escapeChars.contains(input[position]))
                return logger.error(location, "Invalid escape sequence", 0);

            position++;
            return escapeChars.at(input[position]);
        }
        else
        {
            return logger.error(location, "Incomplete escape sequence", 0);
        }
    }
    else if (position < input.length())
    {
        return input[position++];
    }
    else
    {
        return logger.error(location, "Unexpected end of char sequence", 0);
    }
}