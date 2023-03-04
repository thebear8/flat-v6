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
    IRStatement* body;

    IRFunction(
        std::string const& name,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        IRStatement* body
    )
        : name(name), params(params), result(result), body(body)
    {
    }

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
    std::vector<std::pair<std::string, std::vector<IRType*>>> requirements;

    IRFunctionTemplate(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        IRStatement* body
    )
        : IRFunction(name, params, result, body),
          typeParams(typeParams),
          requirements(requirements)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(parent, IRModule*, getParent, setParent)
};

struct IRFunctionInstantiation : IRFunction
{
    std::vector<IRType*> typeArgs;
    std::vector<std::pair<std::string, std::vector<IRType*>>> requirements;

    IRFunctionInstantiation(
        std::string const& name,
        std::vector<IRType*> const& typeArgs,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        IRStatement* body
    )
        : IRFunction(name, params, result, body),
          typeArgs(typeArgs),
          requirements(requirements)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        instantiatedFrom,
        IRFunctionTemplate*,
        getInstantiatedFrom,
        setInstantiatedFrom
    )
};