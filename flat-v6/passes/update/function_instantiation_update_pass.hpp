#pragma once
#include "../../ir/ir.hpp"

class ErrorLogger;
class CompilationContext;
class FunctionInstantiator;
class FunctionBodyInstantiator;
class GraphContext;
class Environment;

class FunctionInstantiationUpdatePass : IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    FunctionInstantiator& m_functionInstantiator;
    FunctionBodyInstantiator& m_functionBodyInstantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    FunctionInstantiationUpdatePass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        FunctionInstantiator& functionInstantiator,
        FunctionBodyInstantiator& functionBodyInstantiator
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_functionInstantiator(functionInstantiator),
          m_functionBodyInstantiator(functionBodyInstantiator)
    {
    }

public:
    void process(IRModule* node);

private:
    void visit(IRModule* node) override;
};