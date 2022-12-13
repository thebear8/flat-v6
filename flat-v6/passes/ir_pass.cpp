#include "ir_pass.hpp"
#include "../util/string_switch.hpp"

IRNode* IRPass::visit(ASTIntegerExpression* node)
{
	auto radix = StringSwitch<size_t>(node->value)
		.StartsWith("0x", 16)
		.StartsWith("0b", 2)
		.Default(10);

	auto value = StringSwitch<std::string>(node->value)
		.StartsWith("0x", node->value.substr(2))
		.StartsWith("0b", node->value.substr(2))
		.Default(node->value);

	auto width = StringSwitch<size_t>(node->suffix)
		.EndsWith("8", 8)
		.EndsWith("16", 16)
		.EndsWith("32", 32)
		.EndsWith("64", 64)
		.OrThrow();

	auto isSigned = StringSwitch<bool>(node->suffix)
		.StartsWith("u", false)
		.StartsWith("i", false)
		.OrThrow();

	return irCtx.make(IRIntegerExpression(isSigned, width, radix, value));
}

IRNode* IRPass::visit(ASTBoolExpression* node)
{
	auto value = StringSwitch<bool>(node->value)
		.Case("true", true)
		.Case("false", false)
		.OrThrow();

	return irCtx.make(IRBoolExpression(value));
}

IRNode* IRPass::visit(ASTCharExpression* node)
{
	size_t position = 0;
	return irCtx.make(
		IRCharExpression(
			unescapeCodePoint(node->value, position, node)
		));
}

IRNode* IRPass::visit(ASTStringExpression* node)
{
	return irCtx.make(
		IRStringExpression(
			unescapeStringUTF8(node->value, node)
		));
}

IRNode* IRPass::visit(ASTIdentifierExpression* node)
{
	return irCtx.make(IRIdentifierExpression(node->value));
}

IRNode* IRPass::visit(ASTStructExpression* node)
{
	std::vector<std::pair<std::string, IRExpression*>> fields;
	for (auto const& [name, value] : node->fields)
		fields.push_back({ name, (IRExpression*)dispatch(value) });

	return irCtx.make(IRStructExpression(node->structName, fields));
}

IRNode* IRPass::visit(ASTUnaryExpression* node)
{
	return irCtx.make(
		IRUnaryExpression(
			node->operation,
			(IRExpression*)dispatch(node->expression)
		));
}

IRNode* IRPass::visit(ASTBinaryExpression* node)
{
	return irCtx.make(
		IRBinaryExpression(
			node->operation,
			(IRExpression*)dispatch(node->left),
			(IRExpression*)dispatch(node->right)
		));
}

IRNode* IRPass::visit(ASTCallExpression* node)
{
	std::vector<IRExpression*> args;
	for (auto arg : node->args)
		args.push_back((IRExpression*)dispatch(arg));

	return irCtx.make(
		IRCallExpression(
			(IRExpression*)dispatch(node->expression),
			args
		));
}

IRNode* IRPass::visit(ASTIndexExpression* node)
{
	std::vector<IRExpression*> args;
	for (auto arg : node->args)
		args.push_back((IRExpression*)dispatch(arg));

	return irCtx.make(
		IRIndexExpression(
			(IRExpression*)dispatch(node->expression),
			args
		));
}

IRNode* IRPass::visit(ASTFieldExpression* node)
{
	return irCtx.make(
		IRFieldExpression(
			(IRExpression*)dispatch(node->expression),
			node->fieldName
		));
}

IRNode* IRPass::visit(ASTBlockStatement* node)
{
	std::vector<IRStatement*> statements;
	for (auto statement : node->statements)
		statements.push_back((IRStatement*)dispatch(statement));

	return irCtx.make(IRBlockStatement(statements));
}

IRNode* IRPass::visit(ASTExpressionStatement* node)
{
	return irCtx.make(
		IRExpressionStatement(
			(IRExpression*)dispatch(node->expression)
		));
}

IRNode* IRPass::visit(ASTVariableStatement* node)
{
	std::vector<std::pair<std::string, IRExpression*>> items;
	for (auto const& [name, value] : node->items)
		items.push_back({ name, (IRExpression*)dispatch(value) });

	return irCtx.make(IRVariableStatement(items));
}

IRNode* IRPass::visit(ASTReturnStatement* node)
{
	return irCtx.make(
		IRReturnStatement(
			(IRExpression*)dispatch(node->expression)
		));
}

IRNode* IRPass::visit(ASTWhileStatement* node)
{
	return irCtx.make(
		IRWhileStatement(
			(IRExpression*)dispatch(node->condition),
			(IRStatement*)dispatch(node->body)
		));
}

IRNode* IRPass::visit(ASTIfStatement* node)
{
	return irCtx.make(
		IRIfStatement(
			(IRExpression*)dispatch(node->condition),
			(IRStatement*)dispatch(node->ifBody),
			(IRStatement*)((node->elseBody) ? dispatch(node->elseBody) : nullptr)
		));
}

IRNode* IRPass::visit(ASTStructDeclaration* node)
{
	std::vector<std::pair<std::string, Type*>> fields;
	for (auto const& [name, type] : node->fields)
		fields.push_back({ name, modCtx.getType(type) });

	return irCtx.make(IRStructDeclaration(node->name, fields));
}

IRNode* IRPass::visit(ASTFunctionDeclaration* node)
{
	std::vector<std::pair<std::string, Type*>> params;
	for (auto const& [name, type] : node->parameters)
		params.push_back({ name, modCtx.getType(type) });

	return irCtx.make(
		IRFunctionDeclaration(
			node->name,
			modCtx.getType(node->result),
			params,
			(IRStatement*)dispatch(node->body)
		));
}

IRNode* IRPass::visit(ASTExternFunctionDeclaration* node)
{
	std::vector<std::pair<std::string, Type*>> params;
	for (auto const& [name, type] : node->parameters)
		params.push_back({ name, modCtx.getType(type) });

	return irCtx.make(
		IRExternFunctionDeclaration(
			node->lib,
			node->name,
			modCtx.getType(node->result),
			params
		));
}

IRNode* IRPass::visit(ASTSourceFile* node)
{
	std::vector<IRDeclaration*> declarations;
	for (auto declaration : node->declarations)
		declarations.push_back((IRDeclaration*)dispatch(declaration));

	return irCtx.make(
		IRSourceFile(
			node->modulePath, 
			node->importPaths,
			declarations
		)); 
}

std::vector<uint8_t> IRPass::unescapeStringUTF8(std::string const& input, ASTNode* node)
{
	std::vector<uint8_t> bytes;

	size_t position = 0;
	while (position < input.length())
	{
		uint32_t cp = unescapeCodePoint(input, position, node);
		if (cp < 0x7F)
		{
			bytes.push_back(cp);
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
			return logger.error(node, "Invalid Unicode code point", bytes);
		}
	}

	bytes.push_back(0);
	return bytes;
}

uint32_t IRPass::unescapeCodePoint(std::string const& input, size_t& position, ASTNode* node)
{
	if (position < input.length() && input[position] == '\\')
	{
		position++;
		if (position < input.length() && isDigit(input[position])) // octal char literal
		{
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) > 3)
				return logger.error(node, "Octal char literal cannot have more than three digits", 0);

			return std::stoul(input.substr(start, (position - start)), nullptr, 8);
		}
		else if (position < input.length() && input[position] == 'x') // hex char literal
		{
			position++;
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) == 0)
				return logger.error(node, "Hex char literal cannot have zero digits", 0);

			return std::stoul(input.substr(start, (position - start)), nullptr, 16);
		}
		else if (position < input.length() && input[position] == 'u') // 0xhhhh unicode code point
		{
			position++;
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) != 4)
				logger.error(node, "2 byte Unicode code point (\\u) must have 4 digits", "");

			return std::stoul(input.substr(start, (position - start)), nullptr, 16);
		}
		else if (position < input.length() && input[position] == 'U') // 0xhhhhhhhh unicode code point
		{
			position++;
			size_t start = position;
			while (position < input.length() && isDigit(input[position]))
				position++;

			if ((position - start) != 8)
				logger.error(node, "4 byte Unicode code point (\\U) must have 8 digits", "");

			return std::stoul(input.substr(start, (position - start)), nullptr, 16);
		}
		else if (position < input.length())
		{
			if (!escapeChars.contains(input[position]))
				return logger.error(node, "Invalid escape sequence", 0);

			position++;
			return escapeChars.at(input[position]);
		}
		else
		{
			return logger.error(node, "Incomplete escape sequence", 0);
		}
	}
	else if (position < input.length())
	{
		return input[position++];
	}
	else
	{
		return logger.error(node, "Unexpected end of char sequence", 0);
	}
}