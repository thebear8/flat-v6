#pragma once
#include <string>
#include <vector>

#include "ir_node.hpp"

struct IRConstraint : IRNode
{
    std::string name;
    std::vector<IRGenericType*> typeParams;
    std::vector<std::pair<std::string, std::vector<IRType*>>> requirements;
    std::vector<IRFunction*> conditions;

    IRConstraint(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        std::vector<IRFunction*> const& conditions
    )
        : name(name),
          typeParams(),
          requirements(requirements),
          conditions(conditions)
    {
    }

    IMPLEMENT_ACCEPT()
};
