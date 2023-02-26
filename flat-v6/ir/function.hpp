#pragma once
#include <string>
#include <vector>

#include "ir_node.hpp"

namespace llvm
{
class Function;
};

struct IRFunction : public IRNode
{
    std::string lib, name;
    std::vector<IRGenericType*> typeParams;
    std::vector<std::pair<std::string, std::vector<IRType*>>> requirements;
    std::vector<std::pair<std::string, IRType*>> params;
    IRType* result;
    IRStatement* body;

    IRFunction(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        IRStatement* body
    )
        : name(name),
          typeParams(typeParams),
          requirements(requirements),
          params(params),
          result(result),
          body(body)
    {
    }

    IRFunction(
        std::string const& lib,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, std::vector<IRType*>>> const&
            requirements,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result
    )
        : lib(lib),
          name(name),
          typeParams(typeParams),
          requirements(requirements),
          result(result),
          params(params),
          body(nullptr)
    {
    }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        llvmFunction, llvm::Function*, getLLVMFunction, setLLVMFunction
    )
};