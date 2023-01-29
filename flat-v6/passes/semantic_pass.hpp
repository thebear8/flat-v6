#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../compiler.hpp"
#include "../data/ir.hpp"
#include "../data/operator.hpp"
#include "../util/error_logger.hpp"

class SemanticPass : IRVisitor<Type*>
{
private:
    ErrorLogger& logger;
    CompilationContext& compCtx;
    ModuleContext& modCtx;

    Type *functionResult, *expectedFunctionResult;
    std::unordered_map<std::string, Type*> localVariables;

public:
    SemanticPass(
        ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx)
        : logger(logger),
          compCtx(compCtx),
          modCtx(modCtx),
          functionResult(nullptr),
          expectedFunctionResult(nullptr)
    {
    }

public:
    void analyze(IRSourceFile* source);

private:
    virtual Type* visit(IRIntegerExpression* node) override;
    virtual Type* visit(IRBoolExpression* node) override;
    virtual Type* visit(IRCharExpression* node) override;
    virtual Type* visit(IRStringExpression* node) override;
    virtual Type* visit(IRIdentifierExpression* node) override;
    virtual Type* visit(IRStructExpression* node) override;
    virtual Type* visit(IRUnaryExpression* node) override;
    virtual Type* visit(IRBinaryExpression* node) override;
    virtual Type* visit(IRCallExpression* node) override;
    virtual Type* visit(IRIndexExpression* node) override;
    virtual Type* visit(IRFieldExpression* node) override;

    virtual Type* visit(IRBlockStatement* node) override;
    virtual Type* visit(IRExpressionStatement* node) override;
    virtual Type* visit(IRVariableStatement* node) override;
    virtual Type* visit(IRReturnStatement* node) override;
    virtual Type* visit(IRWhileStatement* node) override;
    virtual Type* visit(IRIfStatement* node) override;

    virtual Type* visit(IRStructDefinition* node) override;
    virtual Type* visit(IRFunctionDefinition* node) override;
    virtual Type* visit(IRSourceFile* node) override;
};