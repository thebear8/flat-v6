#pragma once
#include <string>
#include <string_view>
#include <ostream>
#include <unordered_map>

#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include "../data/ast.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"

class LLVMCodegenPass : protected Visitor<llvm::Value*>
{
private:
	ErrorLogger& logger;
	TypeContext& typeCtx;

	llvm::LLVMContext& llvmCtx;
	llvm::Module& mod;
	llvm::IRBuilder<> builder;

	bool isFunctionBodyPass;
	std::unordered_map<Type*, llvm::Type*> llvmTypes;
	std::unordered_map<std::string, llvm::Value*> localValues;

public:
	LLVMCodegenPass(ErrorLogger& logger, TypeContext& ctx, llvm::LLVMContext& llvmCtx, llvm::Module& mod) :
		logger(logger), typeCtx(ctx), llvmCtx(llvmCtx), mod(mod), builder(llvmCtx), isFunctionBodyPass(false) { }

public:
	void compile(ASTNode* ast);
	void optimize();

protected:
	virtual llvm::Value* visit(ASTIntegerExpression* node) override;
	virtual llvm::Value* visit(ASTBoolExpression* node) override;
	virtual llvm::Value* visit(ASTCharExpression* node) override;
	virtual llvm::Value* visit(ASTStringExpression* node) override;
	virtual llvm::Value* visit(ASTIdentifierExpression* node) override;
	virtual llvm::Value* visit(ASTStructExpression* node) override;
	virtual llvm::Value* visit(ASTUnaryExpression* node) override;
	virtual llvm::Value* visit(ASTBinaryExpression* node) override;
	virtual llvm::Value* visit(ASTBoundCallExpression* node) override;
	virtual llvm::Value* visit(ASTBoundIndexExpression* node) override;
	virtual llvm::Value* visit(ASTFieldExpression* node) override;

	virtual llvm::Value* visit(ASTBlockStatement* node) override;
	virtual llvm::Value* visit(ASTExpressionStatement* node) override;
	virtual llvm::Value* visit(ASTVariableStatement* node) override;
	virtual llvm::Value* visit(ASTReturnStatement* node) override;
	virtual llvm::Value* visit(ASTWhileStatement* node) override;
	virtual llvm::Value* visit(ASTIfStatement* node) override;

	virtual llvm::Value* visit(ASTStructDeclaration* node) override;
	virtual llvm::Value* visit(ASTFunctionDeclaration* node) override;
	virtual llvm::Value* visit(ASTExternFunctionDeclaration* node) override;
	virtual llvm::Value* visit(ASTSourceFile* node) override;

private:
	llvm::Type* getLLVMType(Type* type);
	std::string getMangledType(Type* type);
	std::string getMangledFunction(std::string const& function, std::vector<Type*> const& params);

private:
	std::string unescapeString(std::string const& input, ASTNode* node);
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