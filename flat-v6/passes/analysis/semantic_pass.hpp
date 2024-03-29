#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../ir/ir.hpp"
#include "../../util/optional_ref.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class Instantiator;
class CallTargetResolver;
class Formatter;
class StructInstantiationUpdatePass;

class SemanticPass : IRVisitor<IRType*>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    GraphContext& m_envCtx;
    Instantiator& m_instantiator;
    CallTargetResolver& m_callTargetResolver;
    Formatter& m_formatter;
    StructInstantiationUpdatePass& m_structUpdatePass;

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
        CallTargetResolver& callTargetResolver,
        Formatter& formatter,
        StructInstantiationUpdatePass& structUpdatePass
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_envCtx(envCtx),
          m_instantiator(instantiator),
          m_callTargetResolver(callTargetResolver),
          m_formatter(formatter),
          m_structUpdatePass(structUpdatePass)
    {
    }

public:
    void process(IRModule* node);

private:
    virtual IRType* visit(IRIntegerExpression* node) override;
    virtual IRType* visit(IRBoolExpression* node) override;
    virtual IRType* visit(IRCharExpression* node) override;
    virtual IRType* visit(IRStringExpression* node) override;
    virtual IRType* visit(IRIdentifierExpression* node) override;
    virtual IRType* visit(IRStructExpression* node) override;
    virtual IRType* visit(IRLoweredCallExpression* node, IRNode*& ref) override;
    virtual IRType* visit(IRFieldExpression* node) override;

    virtual IRType* visit(IRBlockStatement* node) override;
    virtual IRType* visit(IRExpressionStatement* node) override;
    virtual IRType* visit(IRVariableStatement* node) override;
    virtual IRType* visit(IRReturnStatement* node) override;
    virtual IRType* visit(IRWhileStatement* node) override;
    virtual IRType* visit(IRIfStatement* node) override;

    virtual IRType* visit(IRNormalFunction* node) override;

private:
    IRFunction* findCallTarget(
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<IRType*> const& args,
        IRType* result = nullptr,
        optional_ref<std::string> reason = std::nullopt
    );
};