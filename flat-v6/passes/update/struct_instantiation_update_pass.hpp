#pragma once
#include "../../ir/ir.hpp"

class GraphContext;
class Environment;
class Instantiator;

class StructInstantiationUpdatePass
{
private:
    GraphContext& m_envCtx;
    Instantiator& m_instantiator;

    Environment* m_env = nullptr;
    GraphContext* m_irCtx = nullptr;

public:
    StructInstantiationUpdatePass(
        GraphContext& envCtx, Instantiator& instantiator
    )
        : m_envCtx(envCtx), m_instantiator(instantiator)
    {
    }

public:
    void process(IRModule* node);
    IRStructInstantiation* update(IRStructInstantiation* node);
};