#pragma once
#include "lexer.hpp"
#include "../data/ast.hpp"

class Parser : protected Lexer
{
private:
	ErrorLogger& logger;
	AstContext& ctx;

public:
	Parser(ErrorLogger& logger, AstContext& ctx, std::string_view input) :
		Lexer(logger, input), logger(logger), ctx(ctx) { }

public:
	ASTExpression* l0();
	ASTExpression* l1();
	ASTExpression* l2();
	ASTExpression* l3();
	ASTExpression* l4();
	ASTExpression* l5();
	ASTExpression* l6();
	ASTExpression* l7();
	ASTExpression* l8();
	ASTExpression* l9();
	ASTExpression* l10();
	ASTExpression* expression();

	ASTStatement* blockStatement(size_t begin);
	ASTStatement* variableStatement(size_t begin);
	ASTStatement* returnStatement(size_t begin);
	ASTStatement* whileStatement(size_t begin);
	ASTStatement* ifStatement(size_t begin);
	ASTStatement* statement();

	ASTStructDeclaration* structDeclaration(size_t begin);
	ASTFunctionDeclaration* functionDeclaration(size_t begin);
	ASTExternFunctionDeclaration* externFunctionDeclaration(size_t begin);

	ASTSourceFile* sourceFile();

	ASTType* typeName();
};