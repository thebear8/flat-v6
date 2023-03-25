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

struct IRFunctionHead : public IRNode
{
    std::string name;
    std::vector<std::pair<std::string, IRType*>> params;
    IRType* result;

    IRFunctionHead(
        std::string const& name,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result
    )
        : name(name), params(params), result(result)
    {
    }

    virtual bool isFunctionHead() { return true; }
    virtual bool isFunctionTemplate() { return false; }
    virtual bool isFunctionInstantiation() { return false; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        libraryNameForImport,
        std::string,
        getLibraryNameForImport,
        setLibraryNameForImport
    )
};

struct IRFunctionTemplate : IRFunctionHead
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
        : IRFunctionHead(name, params, result),
          typeParams(typeParams),
          requirements(requirements),
          body(body)
    {
    }

    virtual bool isFunctionTemplate() override { return true; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(parent, IRModule*, getParent, setParent)
};

struct IRFunctionInstantiation : IRFunctionHead
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
        : IRFunctionHead(name, params, result),
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

    METADATA_PROP(
        llvmFunction, llvm::Function*, getLLVMFunction, setLLVMFunction
    )
};