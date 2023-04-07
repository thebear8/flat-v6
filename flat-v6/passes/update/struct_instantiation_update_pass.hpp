#pragma once
#include "../../ir/ir.hpp"

class ErrorLogger;
class CompilationContext;
class Instantiator;
class GraphContext;
class Environment;

class StructInstantiationUpdatePass : IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    Instantiator& m_instantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    StructInstantiationUpdatePass(
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