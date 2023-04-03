#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../compiler.hpp"
#include "../../data/operator.hpp"
#include "../../ir/ir.hpp"
#include "../../util/error_logger.hpp"

class LLVMCodegenPass : protected IRVisitor<llvm::Value*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    llvm::LLVMContext& m_llvmCtx;
    llvm::Module& m_llvmModule;
    llvm::IRBuilder<> m_builder;

    std::unordered_map<IRType*, llvm::Type*> m_llvmTypes;
    std::unordered_map<std::string, llvm::Value*> m_localValues;

public:
    LLVMCodegenPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        llvm::LLVMContext& llvmCtx,
        llvm::Module& llvmModule
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_llvmCtx(llvmCtx),
          m_llvmModule(m_llvmModule),
          m_builder(llvmCtx)
    {
    }

public:
    void process(IRModule* node);

private:
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

    virtual llvm::Value* visit(IRFunctionInstantiation* node) override;
    virtual llvm::Value* visit(IRModule* node) override;

private:
    llvm::Type* getLLVMType(IRType* type);
    std::string getMangledTypeName(IRType* type);
    std::string getMangledFunctionName(
        std::string const& function, std::vector<IRType*> const& params
    );
};