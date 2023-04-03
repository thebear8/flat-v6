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
    Instantiator m_instantiator;

    IRModule* m_module;
    GraphContext* m_irCtx;
    Environment* m_env;

public:
    StructInstantiationFixupPass(
        ErrorLogger& logger, CompilationContext& compCtx
    )
        : m_logger(logger),
          m_compCtx(compCtx),
          m_module(nullptr),
          m_env(nullptr)
    {
    }

public:
    void process(IRModule* node);

private:
    void visit(IRModule* node) override;
};