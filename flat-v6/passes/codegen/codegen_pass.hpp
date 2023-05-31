#pragma once

#include <llvm/IR/IRBuilder.h>

#include <ostream>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../data/operator.hpp"
#include "../../ir/ir.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class Environment;

namespace llvm
{
class LLVMContext;
class Module;
class Value;
class Type;

// llvm::IRBuilder<> can't be forward declared because of default template args
}

class LLVMCodegenPass : protected IRVisitor<llvm::Value*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_envCtx;

    llvm::LLVMContext& m_llvmCtx;
    llvm::Module& m_llvmModule;
    llvm::IRBuilder<>& m_builder;

    std::unordered_map<IRType*, llvm::Type*> m_llvmTypes;

    std::stack<std::vector<IRExpression*>> m_args;
    Environment* m_env = nullptr;

public:
    LLVMCodegenPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        GraphContext& envCtx,
        llvm::LLVMContext& llvmCtx,
        llvm::Module& llvmModule,
        llvm::IRBuilder<>& llvmBuilder
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_envCtx(envCtx),
          m_llvmCtx(llvmCtx),
          m_llvmModule(llvmModule),
          m_builder(llvmBuilder)
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
    virtual llvm::Value* visit(IRBoundCallExpression* node) override;
    virtual llvm::Value* visit(IRFieldExpression* node) override;

    virtual llvm::Value* visit(IRBlockStatement* node) override;
    virtual llvm::Value* visit(IRExpressionStatement* node) override;
    virtual llvm::Value* visit(IRVariableStatement* node) override;
    virtual llvm::Value* visit(IRReturnStatement* node) override;
    virtual llvm::Value* visit(IRWhileStatement* node) override;
    virtual llvm::Value* visit(IRIfStatement* node) override;

    virtual llvm::Value* visit(IRIntrinsicFunction* node) override;
    virtual llvm::Value* visit(IRNormalFunction* node) override;

private:
    void generateFunctionHead(IRFunction* function);
    void generateFunctionBody(IRFunction* function);

    llvm::Value* generateUnaryIntrinsic(IRIntrinsicFunction* node);
    llvm::Value* generateBinaryIntrinsic(IRIntrinsicFunction* node);

    llvm::Type* getLLVMType(IRType* type);
    std::string getMangledTypeName(IRType* type);
    std::string getMangledFunctionName(
        std::string const& function, std::vector<IRType*> const& params
    );

private:
    const std::set<std::string> UNARY_INTRINSICS = {
        "__pos__", "__neg__", "__not__", "__lnot__"
    };

    const std::set<std::string> BINARY_INTRINSICS = {
        "__add__", "__sub__", "__mul__", "__div__", "__mod__",  "__and__",
        "__or__",  "__xor__", "__shl__", "__shr__", "__land__", "__lor__",
        "__eq__",  "__ne__",  "__lt__",  "__gt__",  "__lteq__", "__gteq__"
    };
};