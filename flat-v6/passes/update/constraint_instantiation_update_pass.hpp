#pragma once
#include "../../ir/ir.hpp"

class GraphContext;
class Environment;
class Instantiator;

class ConstraintInstantiationUpdatePass
{
private:
    GraphContext& m_envCtx;
    Instantiator& m_instantiator;

    IRModule* m_module = nullptr;
    GraphContext* m_irCtx = nullptr;
    Environment* m_env = nullptr;

public:
    ConstraintInstantiationUpdatePass(
        GraphContext& envCtx, Instantiator& instantiator
    )
        : m_envCtx(envCtx), m_instantiator(instantiator)
    {
    }

public:
    void process(IRModule* node);
    IRConstraintInstantiation* update(
        IRConstraintInstantiation* constraintInstantiation
    );
};