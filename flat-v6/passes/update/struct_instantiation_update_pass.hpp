#pragma once
#include "../../ir/ir.hpp"

class ErrorLogger;
class CompilationContext;
class StructInstantiator;
class GraphContext;
class Environment;

class StructInstantiationUpdatePass : IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    StructInstantiator& m_structInstantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    StructInstantiationUpdatePass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        StructInstantiator& structInstantiator
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_structInstantiator(structInstantiator)
    {
    }

public:
    void process(IRModule* node);

private:
    void visit(IRModule* node) override;
};