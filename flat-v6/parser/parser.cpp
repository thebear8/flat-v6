#include "parser.hpp"

Expression* Parser::l0()
{
	auto begin = trim();
	if (match(Token::ParenOpen)) {
		auto e = expression();
		expect(Token::ParenClose);
		return e;
	}
	else if (match(Token::Integer)) {
		return ctx.make<IntegerExpression>(begin, position, getIntValue(), getIntSuffixValue());
	}
	else if (match(Token::True) || match(Token::False)) {
		return ctx.make<BoolExpression>(begin, position, getTokenValue());
	}
	else if (match(Token::CharLiteral)) {
		return ctx.make<CharExpression>(begin, position, getTokenValue());
	}
	else if (match(Token::StringLiteral)) {
		return ctx.make<StringExpression>(begin, position, getTokenValue());
	}
	else if (match(Token::Identifier)) {
		auto identifier = getTokenValue();
		if (match(Token::BraceOpen)) {
			std::vector<std::pair<std::string, Expression*>> values;
			while (!match(Token::BraceClose) && !match(Token::Eof)) {
				expect(Token::Identifier);
				auto name = getTokenValue();
				expect(Token::Colon);
				values.push_back(std::pair(name, expression()));
				if (!match(Token::Comma)) {
					expect(Token::BraceClose);
					break;
				}
			}
			return ctx.make<StructExpression>(begin, position, identifier, values);
		}
		else {
			return ctx.make<IdentifierExpression>(begin, position, identifier);
		}
	}
	else {
		logger.error(position, "Invalid L0");
		return nullptr;
	}
}

Expression* Parser::l1()
{
	auto begin = trim();
	auto e = l0();
	while (true) {
		if (match(Token::ParenOpen)) {
			std::vector<Expression*> args;
			while (!match(Token::ParenClose) && !match(Token::Eof)) {
				args.push_back(expression());
				match(Token::Comma);
			}
			e = ctx.make<CallExpression>(begin, position, e, args);
		}
		else if (match(Token::BracketOpen)) {
			std::vector<Expression*> args;
			while (!match(Token::BracketClose) && !match(Token::Eof)) {
				args.push_back(expression());
				match(Token::Comma);
			}
			e = ctx.make<IndexExpression>(begin, position, e, args);
		}
		else if (match(Token::Dot)) {
			expect(Token::Identifier);
			e = ctx.make<FieldExpression>(begin, position, e, getTokenValue());
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l2()
{
	auto begin = trim();
	if (match(Token::Plus)) {
		auto e = l2();
		return ctx.make<UnaryExpression>(begin, position, UnaryOperator::Positive, e);
	}
	else if (match(Token::Minus)) {
		auto e = l2();
		return ctx.make<UnaryExpression>(begin, position, UnaryOperator::Negative, e);
	}
	else if (match(Token::LogicalNot)) {
		auto e = l2();
		return ctx.make<UnaryExpression>(begin, position, UnaryOperator::LogicalNot, e);
	}
	else if (match(Token::BitwiseNot)) {
		auto e = l2();
		return ctx.make<UnaryExpression>(begin, position, UnaryOperator::BitwiseNot, e);
	}
	else {
		return l1();
	}
}

Expression* Parser::l3()
{
	auto begin = trim();
	auto e = l2();
	while (true) {
		if (match(Token::Multiply)) {
			auto r = l2();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::Multiply, e, r);
		}
		else if (match(Token::Divide)) {
			auto r = l2();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::Divide, e, r);
		}
		else if (match(Token::Modulo)) {
			auto r = l2();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::Modulo, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l4()
{
	auto begin = trim();
	auto e = l3();
	while (true) {
		if (match(Token::Plus)) {
			auto r = l3();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::Add, e, r);
		}
		else if (match(Token::Minus)) {
			auto r = l3();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::Subtract, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l5()
{
	auto begin = trim();
	auto e = l4();
	while (true) {
		if (match(Token::ShiftLeft)) {
			auto r = l4();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::ShiftLeft, e, r);
		}
		else if (match(Token::ShiftRight)) {
			auto r = l4();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::ShiftRight, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l6()
{
	auto begin = trim();
	auto e = l5();
	while (true) {
		if (match(Token::BitwiseAnd)) {
			auto r = l5();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::BitwiseAnd, e, r);
		}
		else if (match(Token::BitwiseOr)) {
			auto r = l5();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::BitwiseOr, e, r);
		}
		else if (match(Token::BitwiseXor)) {
			auto r = l5();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::BitwiseXor, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l7()
{
	auto begin = trim();
	auto e = l6();
	while (true) {
		if (match(Token::Equal)) {
			auto r = l6();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::Equal, e, r);
		}
		else if (match(Token::NotEqual)) {
			auto r = l6();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::NotEqual, e, r);
		}
		else if (match(Token::LessThan)) {
			auto r = l6();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::LessThan, e, r);
		}
		else if (match(Token::GreaterThan)) {
			auto r = l6();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::GreaterThan, e, r);
		}
		else if (match(Token::LessOrEqual)) {
			auto r = l6();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::LessOrEqual, e, r);
		}
		else if (match(Token::GreaterOrEqual)) {
			auto r = l6();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::GreaterOrEqual, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l8()
{
	auto begin = trim();
	auto e = l7();
	while (true) {
		if (match(Token::LogicalAnd)) {
			auto r = l7();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::LogicalAnd, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l9()
{
	auto begin = trim();
	auto e = l8();
	while (true) {
		if (match(Token::LogicalOr)) {
			auto r = l8();
			e = ctx.make<BinaryExpression>(begin, position, BinaryOperator::LogicalOr, e, r);
		}
		else {
			return e;
		}
	}
}

Expression* Parser::l10()
{
	auto begin = trim();
	auto e = l9();
	if (match(Token::Assign)) {
		auto r = l10();
		return ctx.make<BinaryExpression>(begin, position, BinaryOperator::Assign, e, r);
	}
	else {
		return e;
	}
}

Expression* Parser::expression()
{
	return l10();
}

Statement* Parser::blockStatement(size_t begin)
{
	std::vector<Statement*> statements;
	while (!match(Token::BraceClose) && !match(Token::Eof)) {
		statements.push_back(statement());
	}
	return ctx.make<BlockStatement>(begin, position, statements);
}

Statement* Parser::variableStatement(size_t begin)
{
	std::vector<std::pair<std::string, Expression*>> items;
	while (match(Token::Identifier)) {
		auto name = getTokenValue();
		expect(Token::Assign);
		items.push_back(std::pair(name, expression()));
		match(Token::Comma);
	}
	return ctx.make<VariableStatement>(begin, position, items);
}

Statement* Parser::returnStatement(size_t begin)
{
	if (match(Token::NewLine)) {
		return ctx.make<ReturnStatement>(begin, position, nullptr);
	}
	else {
		auto e = expression();
		return ctx.make<ReturnStatement>(begin, position, e);
	}
}

Statement* Parser::whileStatement(size_t begin)
{
	expect(Token::ParenOpen);
	auto condition = expression();
	expect(Token::ParenClose);
	auto body = statement();
	return ctx.make<WhileStatement>(begin, position, condition, body);
}

Statement* Parser::ifStatement(size_t begin)
{
	expect(Token::ParenOpen);
	auto condition = expression();
	expect(Token::ParenClose);
	auto ifBody = statement();
	if (match(Token::Else)) {
		auto elseBody = statement();
		return ctx.make<IfStatement>(begin, position, condition, ifBody, elseBody);
	}
	else {
		return ctx.make<IfStatement>(begin, position, condition, ifBody, nullptr);
	}
}

Statement* Parser::statement()
{
	auto begin = trim();
	if (match(Token::BraceOpen)) {
		return blockStatement(begin);
	}
	else if (match(Token::Let)) {
		return variableStatement(begin);
	}
	else if (match(Token::Return)) {
		return returnStatement(begin);
	}
	else if (match(Token::While)) {
		return whileStatement(begin);
	}
	else if (match(Token::If)) {
		return ifStatement(begin);
	}
	else {
		auto e = expression();
		return ctx.make<ExpressionStatement>(begin, position, e);
	}
}

StructDeclaration* Parser::structDeclaration(size_t begin)
{
	expect(Token::Identifier);
	auto structName = getTokenValue();
	expect(Token::BraceOpen);
	std::vector<std::pair<std::string, Type*>> fields;
	while (!match(Token::BraceClose) && !match(Token::Eof)) {
		expect(Token::Identifier);
		auto name = getTokenValue();
		expect(Token::Colon);
		fields.push_back(std::pair(name, typeName()));
		if (!match(Token::Comma)) {
			expect(Token::BraceClose);
			break;
		}
	}
	return ctx.make<StructDeclaration>(begin, position, structName, fields);
}

FunctionDeclaration* Parser::functionDeclaration(size_t begin)
{
	expect(Token::Identifier);
	auto name = getTokenValue();
	std::vector<std::pair<std::string, Type*>> parameters;
	expect(Token::ParenOpen);
	while (!match(Token::ParenClose) && !match(Token::Eof)) {
		expect(Token::Identifier);
		auto paramName = getTokenValue();
		expect(Token::Colon);
		auto type = typeName();
		parameters.push_back({ paramName, type });
		match(Token::Comma);
	}

	Type* result = typeCtx.getVoid();
	if (match(Token::Colon)) {
		result = typeName();
	}

	auto bodyBegin = trim();
	expect(Token::BraceOpen);
	auto body = blockStatement(bodyBegin);
	return ctx.make<FunctionDeclaration>(begin, position, name, result, parameters, body);
}

ExternFunctionDeclaration* Parser::externFunctionDeclaration(size_t begin)
{
	expect(Token::ParenOpen);
	expect(Token::Identifier);
	auto lib = getTokenValue();
	expect(Token::ParenClose);

	expect(Token::Function);
	expect(Token::Identifier);
	auto name = getTokenValue();
	std::vector<std::pair<std::string, Type*>> parameters;
	expect(Token::ParenOpen);
	while (!match(Token::ParenClose) && !match(Token::Eof)) {
		expect(Token::Identifier);
		auto paramName = getTokenValue();
		expect(Token::Colon);
		auto type = typeName();
		parameters.push_back({ paramName, type });
		match(Token::Comma);
	}

	Type* result = typeCtx.getVoid();
	if (match(Token::Colon)) {
		result = typeName();
	}

	return ctx.make<ExternFunctionDeclaration>(begin, position, lib, name, result, parameters);
}

ParsedSourceFile* Parser::sourceFile()
{
	auto begin = trim();

	std::vector<std::string> modulePath;
	if (match(Token::Module)) {
		while (!match(Token::Eof)) {
			expect(Token::Identifier);
			modulePath.push_back(getTokenValue());
			if (!match(Token::Dot))
				break;
		}
	}

	std::vector<std::vector<std::string>> imports;
	while (match(Token::Import)) {
		std::vector<std::string> importPath;
		while (!match(Token::Eof)) {
			expect(Token::Identifier);
			importPath.push_back(getTokenValue());
			if (match(Token::Dot))
				break;
		}
	}

	std::vector<Declaration*> declarations;
	while (!match(Token::Eof)) {
		auto declBegin = trim();
		if (match(Token::Struct)) {
			declarations.push_back(structDeclaration(declBegin));
		}
		else if (match(Token::Function)) {
			declarations.push_back(functionDeclaration(declBegin));
		}
		else if (match(Token::Extern)) {
			declarations.push_back(externFunctionDeclaration(declBegin));
		}
		else {
			logger.error("Expected eiter StructDeclaration, FunctionDeclaration or ExternFunctionDeclaration");
		}
	}
	return ctx.make<ParsedSourceFile>(begin, position, modulePath, imports, declarations);
}

Type* Parser::typeName()
{
	expect(Token::Identifier);
	Type* type = (typeCtx.getResolvedType(getTokenValue()) ? typeCtx.getResolvedType(getTokenValue()) : typeCtx.getStructType(getTokenValue()));
	while (true) {
		if (match(Token::Multiply)) {
			type = typeCtx.getPointerType(type);
		}
		else if (match(Token::BracketOpen) && match(Token::BracketClose)) {
			type = typeCtx.getArrayType(type);
		}
		else {
			return type;
		}
	}
}
