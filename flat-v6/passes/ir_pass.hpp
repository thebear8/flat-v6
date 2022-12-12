#pragma once
#include "../compiler.hpp"
#include "../data/ast.hpp"
#include "../data/ir.hpp"

class IRPass : public ASTVisitor<IRNode*>
{
private:
	ErrorLogger& logger;
	GraphContext& irCtx;

	virtual IRNode* visit(ASTIntegerExpression* node) override;
	virtual IRNode* visit(ASTBoolExpression* node) override;
	virtual IRNode* visit(ASTCharExpression* node) override;
	virtual IRNode* visit(ASTStringExpression* node) override;
	virtual IRNode* visit(ASTIdentifierExpression* node) override;
	virtual IRNode* visit(ASTStructExpression* node) override;
	virtual IRNode* visit(ASTUnaryExpression* node) override;
	virtual IRNode* visit(ASTBinaryExpression* node) override;
	virtual IRNode* visit(ASTCallExpression* node) override;
	virtual IRNode* visit(ASTBoundCallExpression* node) override;
	virtual IRNode* visit(ASTIndexExpression* node) override;
	virtual IRNode* visit(ASTBoundIndexExpression* node) override;
	virtual IRNode* visit(ASTFieldExpression* node) override;

	virtual IRNode* visit(ASTBlockStatement* node) override;
	virtual IRNode* visit(ASTExpressionStatement* node) override;
	virtual IRNode* visit(ASTVariableStatement* node) override;
	virtual IRNode* visit(ASTReturnStatement* node) override;
	virtual IRNode* visit(ASTWhileStatement* node) override;
	virtual IRNode* visit(ASTIfStatement* node) override;

	virtual IRNode* visit(ASTStructDeclaration* node) override;
	virtual IRNode* visit(ASTFunctionDeclaration* node) override;
	virtual IRNode* visit(ASTExternFunctionDeclaration* node) override;
	virtual IRNode* visit(ASTSourceFile* node) override;

private:
	std::vector<uint8_t> unescapeStringUTF8(std::string const& input, ASTNode* node);
	uint32_t unescapeCodePoint(std::string const& value, size_t& position, ASTNode* node);
	bool isDigit(char c) { return (c >= '0' && c <= '9'); }
	bool isBinaryDigit(char c) { return (c >= '0' && c <= '1'); }
	bool isHexDigit(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
	bool isLetter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	bool isWhitespace(char c) { return (c == ' ' || c == '\t' || c == '\r' || c == '\n'); }
	bool isIdentifier(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'); }

private:
	std::unordered_map<char, uint32_t> escapeChars =
	{
		{ 'a', '\a' },
		{ 'b', '\b' },
		{ 'f', '\f' },
		{ 'n', '\n' },
		{ 'r', '\r' },
		{ 't', '\t' },
		{ 'v', '\v' },
		{ '\\', '\\' },
		{ '\'', '\'' },
		{ '\"', '\"' },
		{ '\?', '\?' },
	};
};