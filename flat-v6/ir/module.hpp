#pragma once
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ir_node.hpp"

class Environment;
class GraphContext;

class IRType;
class IRConstraint;
class IRStructType;
class IRStructInstantiation;
class IRFunction;
class IRFunctionInstantiation;

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

    METADATA_PROP(env, Environment*, getEnv, setEnv)
    METADATA_PROP(irCtx, GraphContext*, getIrCtx, setIrCtx)

    using StructInstantiationMap = std::unordered_map<
        IRStructType*,
        std::map<std::vector<IRType*>, IRStructInstantiation*>>;

    METADATA_PROP(
        structInstantiations,
        StructInstantiationMap,
        getStructInstantiations,
        setStructInstantiations
    )

    using FunctionInstantiationMap = std::unordered_map<
        IRFunction*,
        std::map<std::vector<IRType*>, IRFunctionInstantiation*>>;

    METADATA_PROP(
        functionInstantiations,
        FunctionInstantiationMap,
        getFunctionInstantiations,
        setFunctionInstantiations
    )
};