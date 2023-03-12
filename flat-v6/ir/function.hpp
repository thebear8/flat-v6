#pragma once
#include <string>
#include <vector>

#include "ir_node.hpp"

namespace llvm
{
class Function;
};

struct IRModule;
struct IRType;

struct IRFunction : public IRNode
{
    std::string name;
    std::vector<std::pair<std::string, IRType*>> params;
    IRType* result;

    IRFunction(
        std::string const& name,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result
    )
        : name(name), params(params), result(result)
    {
    }

    virtual bool isBareFunction() { return true; }
    virtual bool isFunctionTemplate() { return false; }
    virtual bool isFunctionInstantiation() { return false; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        libraryNameForImport,
        std::string,
        getLibraryNameForImport,
        setLibraryNameForImport
    )

    METADATA_PROP(
        llvmFunction, llvm::Function*, getLLVMFunction, setLLVMFunction
    )
};

struct IRFunctionTemplate : IRFunction
{
    std::vector<IRGenericType*> typeParams;
    std::set<IRConstraintInstantiation*> requirements;
    IRStatement* body;

    IRFunctionTemplate(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::set<IRConstraintInstantiation*> const& requirements,
        IRStatement* body
    )
        : IRFunction(name, params, result),
          typeParams(typeParams),
          requirements(requirements),
          body(body)
    {
    }

    virtual bool isFunctionTemplate() override { return true; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(parent, IRModule*, getParent, setParent)
};

struct IRFunctionInstantiation : IRFunction
{
    std::vector<IRType*> typeArgs;
    std::set<IRConstraintInstantiation*> requirements;
    IRStatement* body;

    IRFunctionInstantiation(
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::set<IRConstraintInstantiation*> const& requirements,
        IRStatement* body
    )
        : IRFunction(name, params, result),
          typeArgs(typeArgs),
          requirements(requirements),
          body(body)
    {
    }

    virtual bool isFunctionInstantiation() override { return true; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        instantiatedFrom,
        IRFunctionTemplate*,
        getInstantiatedFrom,
        setInstantiatedFrom
    )
};