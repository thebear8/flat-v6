#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../compiler.hpp"
#include "../../data/operator.hpp"
#include "../../ir/ir.hpp"
#include "../../support/formatter.hpp"
#include "../../util/error_logger.hpp"
#include "../../util/optional_ref.hpp"
#include "../support/instantiator.hpp"

class SemanticPass : IRVisitor<IRType*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_envCtx;
    Instantiator& m_instantiator;
    Formatter& m_formatter;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

    IRType* m_result = nullptr;
    IRType* m_expectedResult = nullptr;

public:
    SemanticPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        GraphContext& envCtx,
        Instantiator& instantiator,
        Formatter& formatter
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_envCtx(envCtx),
          m_instantiator(instantiator),
          m_formatter(formatter)
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
    IRFunctionHead* findCallTarget(
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result = nullptr,
        optional_ref<std::string> reason = std::nullopt
    );
};