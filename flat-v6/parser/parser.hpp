#pragma once
#include "lexer.hpp"
#include "../data/ast.hpp"
#include "../type/type.hpp"

class Parser : protected Lexer
{
private:
	ErrorLogger& logger;
	AstContext& ctx;
	TypeContext& typeCtx;

public:
	Parser(ErrorLogger& logger, AstContext& ctx, TypeContext& typeCtx, std::string_view input) :
		Lexer(logger, input), logger(logger), ctx(ctx), typeCtx(typeCtx) { }

public:
	Expression* l0();
	Expression* l1();
	Expression* l2();
	Expression* l3();
	Expression* l4();
	Expression* l5();
	Expression* l6();
	Expression* l7();
	Expression* l8();
	Expression* l9();
	Expression* l10();
	Expression* expression();

	Statement* blockStatement(size_t begin);
	Statement* variableStatement(size_t begin);
	Statement* returnStatement(size_t begin);
	Statement* whileStatement(size_t begin);
	Statement* ifStatement(size_t begin);
	Statement* statement();

	StructDeclaration* structDeclaration(size_t begin);
	FunctionDeclaration* functionDeclaration(size_t begin);
	ExternFunctionDeclaration* externFunctionDeclaration(size_t begin);

	ParsedSourceFile* sourceFile();

	Type* typeName();
};