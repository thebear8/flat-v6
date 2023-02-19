#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../compiler.hpp"
#include "../data/ir.hpp"
#include "../data/operator.hpp"
#include "../util/error_logger.hpp"

class LLVMCodegenPass : protected IRVisitor<llvm::Value*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ModuleContext& m_modCtx;

    llvm::LLVMContext& m_llvmCtx;
    llvm::Module& m_llvmMod;
    llvm::IRBuilder<> m_builder;

    std::unordered_map<IRType*, llvm::Type*> llvmTypes;
    std::unordered_map<std::string, llvm::Value*> localValues;

public:
    LLVMCodegenPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        ModuleContext& modCtx,
        llvm::LLVMContext& llvmCtx,
        llvm::Module& mod
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_modCtx(modCtx),
          m_llvmCtx(llvmCtx),
          m_llvmMod(mod),
          m_builder(llvmCtx)
    {
    }

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
    llvm::Type* getLLVMType(IRType* type);
    std::string getMangledTypeName(IRType* type);
    std::string getMangledFunctionName(
        std::string const& function, std::vector<IRType*> const& params
    );
};