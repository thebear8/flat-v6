#pragma once
#include <string>
#include <string_view>
#include <ostream>
#include <unordered_map>

#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include "../data/ir.hpp"
#include "../util/error_logger.hpp"
#include "../data/operator.hpp"

class LLVMCodegenPass : protected IRVisitor<llvm::Value*>
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
	void process(IRSourceFile* source);
	void optimize();

protected:
	virtual llvm::Value* visit(IRIntegerExpression* node) override;
	virtual llvm::Value* visit(IRBoolExpression* node) override;
	virtual llvm::Value* visit(IRCharExpression* node) override;
	virtual llvm::Value* visit(IRStringExpression* node) override;
	virtual llvm::Value* visit(IRIdentifierExpression* node) override;
	virtual llvm::Value* visit(IRStructExpression* node) override;
	virtual llvm::Value* visit(IRUnaryExpression* node) override;
	virtual llvm::Value* visit(IRBinaryExpression* node) override;
	virtual llvm::Value* visit(IRCallExpression* node) override;
	virtual llvm::Value* visit(IRIndexExpression* node) override;
	virtual llvm::Value* visit(IRFieldExpression* node) override;

	virtual llvm::Value* visit(IRBlockStatement* node) override;
	virtual llvm::Value* visit(IRExpressionStatement* node) override;
	virtual llvm::Value* visit(IRVariableStatement* node) override;
	virtual llvm::Value* visit(IRReturnStatement* node) override;
	virtual llvm::Value* visit(IRWhileStatement* node) override;
	virtual llvm::Value* visit(IRIfStatement* node) override;

	virtual llvm::Value* visit(IRStructDeclaration* node) override;
	virtual llvm::Value* visit(IRFunctionDeclaration* node) override;
	virtual llvm::Value* visit(IRSourceFile* node) override;

private:
	llvm::Type* getLLVMType(Type* type);
	std::string getMangledType(Type* type);
	std::string getMangledFunction(std::string const& function, std::vector<Type*> const& params);
};