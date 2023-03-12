#pragma once
#include <set>
#include <string>
#include <vector>

#include "ir_node.hpp"

struct IRModule;
struct IRFunction;

struct IRConstraint : IRNode
{
    std::string name;
    std::set<IRConstraintInstantiation*> requirements;
    std::vector<IRFunction*> conditions;

    IRConstraint(
        std::string const& name,
        std::set<IRConstraintInstantiation*> const& requirements,
        std::vector<IRFunction*> const& conditions
    )
        : name(name), requirements(requirements), conditions(conditions)
    {
    }

    IMPLEMENT_ACCEPT()
};

struct IRConstraintTemplate : IRConstraint
{
    std::vector<IRGenericType*> typeParams;

    IRConstraintTemplate(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::set<IRConstraintInstantiation*> const& requirements,
        std::vector<IRFunction*> const& conditions
    )
        : IRConstraint(name, requirements, conditions), typeParams(typeParams)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(parent, IRModule*, getParent, setParent)
};

struct IRConstraintInstantiation : IRConstraint
{
    std::vector<IRType*> typeArgs;

    IRConstraintInstantiation(
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::set<IRConstraintInstantiation*> const& requirements,
        std::vector<IRFunction*> const& conditions
    )
        : IRConstraint(name, requirements, conditions), typeArgs(typeArgs)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        instantiatedFrom,
        IRConstraintTemplate*,
        getInstantiatedFrom,
        setInstantiatedFrom
    )
};