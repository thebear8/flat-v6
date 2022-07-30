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

class LLVMCodegenPass : protected Visitor<llvm::Value*>, protected ErrorLogger
{
private:
	TypeContext& typeCtx;
	llvm::LLVMContext& llvmCtx;
	llvm::Module& mod;
	llvm::IRBuilder<> builder;
	std::unordered_map<Type*, llvm::Type*> llvmTypes;
	std::unordered_map<std::string, llvm::Value*> localValues;
	bool isFunctionBodyPass;

public:
	LLVMCodegenPass(TypeContext& ctx, llvm::LLVMContext& llvmCtx, llvm::Module& mod, std::string_view source, std::ostream& logStream) :
		ErrorLogger(source, logStream), typeCtx(ctx), llvmCtx(llvmCtx), mod(mod), builder(llvmCtx), isFunctionBodyPass(false) { }

public:
	void compile(AstNode* ast);
	void optimize();

protected:
	virtual llvm::Value* visit(IntegerExpression* node) override;
	virtual llvm::Value* visit(BoolExpression* node) override;
	virtual llvm::Value* visit(CharExpression* node) override;
	virtual llvm::Value* visit(StringExpression* node) override;
	virtual llvm::Value* visit(IdentifierExpression* node) override;
	virtual llvm::Value* visit(StructExpression* node) override;
	virtual llvm::Value* visit(UnaryExpression* node) override;
	virtual llvm::Value* visit(BinaryExpression* node) override;
	virtual llvm::Value* visit(BoundCallExpression* node) override;
	virtual llvm::Value* visit(BoundIndexExpression* node) override;
	virtual llvm::Value* visit(FieldExpression* node) override;

	virtual llvm::Value* visit(BlockStatement* node) override;
	virtual llvm::Value* visit(ExpressionStatement* node) override;
	virtual llvm::Value* visit(VariableStatement* node) override;
	virtual llvm::Value* visit(ReturnStatement* node) override;
	virtual llvm::Value* visit(WhileStatement* node) override;
	virtual llvm::Value* visit(IfStatement* node) override;

	virtual llvm::Value* visit(StructDeclaration* node) override;
	virtual llvm::Value* visit(FunctionDeclaration* node) override;
	virtual llvm::Value* visit(ExternFunctionDeclaration* node) override;
	virtual llvm::Value* visit(Module* node) override;

	virtual llvm::Value* visit(AstNode* node) override { return node->accept(this); }
	virtual llvm::Value* visit(Expression* node) override { return node->accept(this); }
	virtual llvm::Value* visit(Statement* node) override { return node->accept(this); }
	virtual llvm::Value* visit(Declaration* node) override { return node->accept(this); }
	virtual llvm::Value* visit(CallExpression* node) override { throw std::exception(); }
	virtual llvm::Value* visit(IndexExpression* node) override { throw std::exception(); }

private:
	llvm::Type* getLLVMType(Type* type);
	std::string getMangledType(Type* type);
	std::string getMangledFunction(std::string const& function, std::vector<Type*> const& params);

private:
	std::string unescapeString(std::string const& input, AstNode* node);
	uint32_t unescapeCodePoint(std::string const& value, size_t& position, AstNode* node);
	bool isDigit(char c) { return (c >= '0' && c <= '9'); }
	bool isBinaryDigit(char c) { return (c >= '0' && c <= '1'); }
	bool isHexDigit(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
	bool isLetter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	bool isWhitespace(char c) { return (c == ' ' || c == '\t' || c == '\r' || c == '\n'); }
	bool isIdentifier(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'); }

private:
	void error(AstNode* node, std::string const& message) { return ErrorLogger::error(node->begin, node->end, message); }
	void warning(AstNode* node, std::string const& message) { return ErrorLogger::warning(node->begin, node->end, message); }

	template<typename ReturnType>
	ReturnType error(AstNode* node, std::string const& message, ReturnType&& returnValue)
	{
		error(node, message);
		return std::forward<ReturnType>(returnValue);
	}

	template<typename ReturnType>
	ReturnType warning(AstNode* node, std::string const& message, ReturnType&& returnValue)
	{
		warning(node, message);
		return std::forward<ReturnType>(returnValue);
	}

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