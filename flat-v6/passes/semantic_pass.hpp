#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../compiler.hpp"
#include "../data/ir.hpp"
#include "../data/operator.hpp"
#include "../util/error_logger.hpp"

class SemanticPass : IRVisitor<IRType*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ModuleContext& m_modCtx;

    GraphContext m_envCtx;
    GraphContext m_genericCtx;

    Environment* m_env;

    IRType* m_result;
    IRType* m_expectedResult;

public:
    SemanticPass(
        ErrorLogger& logger, CompilationContext& compCtx, ModuleContext& modCtx
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_modCtx(modCtx),
          m_env(&modCtx),
          m_result(nullptr),
          m_expectedResult(nullptr)
    {
    }

public:
    void analyze(IRSourceFile* source);

private:
    virtual IRType* visit(IRIntegerExpression* node) override;
    virtual IRType* visit(IRBoolExpression* node) override;
    virtual IRType* visit(IRCharExpression* node) override;
    virtual IRType* visit(IRStringExpression* node) override;
    virtual IRType* visit(IRIdentifierExpression* node) override;
    virtual IRType* visit(IRStructExpression* node) override;
    virtual IRType* visit(IRUnaryExpression* node) override;
    virtual IRType* visit(IRBinaryExpression* node) override;
    virtual IRType* visit(IRCallExpression* node) override;
    virtual IRType* visit(IRIndexExpression* node) override;
    virtual IRType* visit(IRFieldExpression* node) override;

    virtual IRType* visit(IRBlockStatement* node) override;
    virtual IRType* visit(IRExpressionStatement* node) override;
    virtual IRType* visit(IRVariableStatement* node) override;
    virtual IRType* visit(IRReturnStatement* node) override;
    virtual IRType* visit(IRWhileStatement* node) override;
    virtual IRType* visit(IRIfStatement* node) override;

    virtual IRType* visit(IRConstraintDeclaration* node) override;
    virtual IRType* visit(IRStructDeclaration* node) override;
    virtual IRType* visit(IRFunctionDeclaration* node) override;
    virtual IRType* visit(IRSourceFile* node) override;

private:
    void setupEnvironment(IRDeclaration* node);
};