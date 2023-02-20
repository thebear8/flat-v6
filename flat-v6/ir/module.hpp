#pragma once
#include <string>
#include <vector>

#include "ir_node.hpp"

struct IRModule : public IRNode
{
    std::string name;
    std::set<std::string> imports;
    std::vector<IRConstraint*> constraints;
    std::vector<IRFunction*> functions;
    std::vector<IRStructType*> structs;

    IRModule(
        std::string const& name,
        std::set<std::string> const& imports,
        std::vector<IRConstraint*> const& constraints,
        std::vector<IRFunction*> const& functions,
        std::vector<IRStructType*> const& structs
    )
        : name(name),
          imports(imports),
          constraints(constraints),
          functions(functions),
          structs(structs)
    {
    }

    IMPLEMENT_ACCEPT()
};