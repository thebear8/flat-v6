#pragma once
#include "../../ir/ir.hpp"

class ErrorLogger;
class CompilationContext;
class ConstraintInstantiator;
class GraphContext;
class Environment;

class ConstraintInstantiationUpdatePass : IRVisitor<void>
{
private:
    ErrorLogger& m_logger;
    CompilationContext& m_compCtx;
    ConstraintInstantiator& m_constraintInstantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    ConstraintInstantiationUpdatePass(
        ErrorLogger& logger,
        CompilationContext& compCtx,
        ConstraintInstantiator& constraintInstantiator
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_constraintInstantiator(constraintInstantiator)
    {
    }

public:
    void process(IRModule* node);

private:
    void visit(IRModule* node) override;
};