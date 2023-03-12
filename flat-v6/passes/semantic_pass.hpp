#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../compiler.hpp"
#include "../data/operator.hpp"
#include "../ir/ir.hpp"
#include "../util/error_logger.hpp"
#include "support/instantiator.hpp"

class SemanticPass : IRVisitor<IRType*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    Instantiator m_instantiator;

    IRModule* m_module;
    GraphContext* m_irCtx;
    Environment* m_env;

    IRType* m_result;
    IRType* m_expectedResult;

public:
    SemanticPass(ErrorLogger& logger, CompilationContext& compCtx)
        : m_logger(logger),
          m_compCtx(compCtx),
          m_module(nullptr),
          m_env(nullptr),
          m_result(nullptr),
          m_expectedResult(nullptr)
    {
    }

public:
    void process(IRModule* mod);

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

    virtual IRType* visit(IRFunctionTemplate* node) override;
    virtual IRType* visit(IRModule* node) override;

private:
    IRFunctionInstantiation* findCallTarget(
        std::string const& name,
        std::unordered_map<IRGenericType*, IRType*>& typeArgs,
        std::vector<IRType*> const& args,
        std::string& error
    );
};