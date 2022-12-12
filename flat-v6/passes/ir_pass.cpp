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
	return irCtx.make(IRCharExpression(unescapeCodePoint(node->value, position, node)));
}

IRNode* IRPass::visit(ASTStringExpression* node)
{
	return irCtx.make(IRStringExpression(unescapeStringUTF8(node->value, node)));
}

IRNode* IRPass::visit(ASTIdentifierExpression* node)
{
	return irCtx.make(IRIdentifierExpression(node->value));
}

IRNode* IRPass::visit(ASTStructExpression* node)
{
	return irCtx.make(IRStructExpression())
}

IRNode* IRPass::visit(ASTUnaryExpression* node)
{

}

IRNode* IRPass::visit(ASTBinaryExpression* node)
{

}

IRNode* IRPass::visit(ASTCallExpression* node)
{

}

IRNode* IRPass::visit(ASTBoundCallExpression* node)
{

}

IRNode* IRPass::visit(ASTIndexExpression* node)
{

}

IRNode* IRPass::visit(ASTBoundIndexExpression* node)
{

}

IRNode* IRPass::visit(ASTFieldExpression* node)
{

}


IRNode* IRPass::visit(ASTBlockStatement* node)
{

}

IRNode* IRPass::visit(ASTExpressionStatement* node)
{

}

IRNode* IRPass::visit(ASTVariableStatement* node)
{

}

IRNode* IRPass::visit(ASTReturnStatement* node)
{

}

IRNode* IRPass::visit(ASTWhileStatement* node)
{

}

IRNode* IRPass::visit(ASTIfStatement* node)
{

}


IRNode* IRPass::visit(ASTStructDeclaration* node)
{

}

IRNode* IRPass::visit(ASTFunctionDeclaration* node)
{

}

IRNode* IRPass::visit(ASTExternFunctionDeclaration* node)
{

}

IRNode* IRPass::visit(ASTSourceFile* node)
{

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