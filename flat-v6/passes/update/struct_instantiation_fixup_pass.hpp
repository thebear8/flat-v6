#pragma once
#include "../../ir/ir.hpp"
#include "../support/instantiator.hpp"

class ErrorLogger;
class CompilationContext;
class GraphContext;
class Environment;

class StructInstantiationFixupPass : IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    Instantiator& m_instantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    StructInstantiationFixupPass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        Instantiator& instantiator
    )
        : m_logger(logger), m_compCtx(compCtx), m_instantiator(instantiator)
    {
    }

public:
    void process(IRModule* node);

private:
    void visit(IRModule* node) override;
};