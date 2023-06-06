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
struct IRConstraintInstantiation;

struct IRFunction : public IRNode
{
    IRModule* parent = nullptr;
    IRFunction* blueprint = nullptr;
    std::string name = {};
    std::vector<IRGenericType*> typeParams = {};
    std::vector<IRType*> typeArgs = {};
    std::vector<std::pair<std::string, IRType*>> params = {};
    IRType* result = nullptr;
    std::vector<IRConstraintInstantiation*> requirements = {};
    IRStatement* body = nullptr;

    IRFunction() {}

    IRFunction(
        IRModule* parent,
        IRFunction* blueprint,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<IRType*> const& typeArgs,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<IRConstraintInstantiation*> requirements,
        IRStatement* body
    )
        : parent(parent),
          blueprint(blueprint),
          name(name),
          typeParams(typeParams),
          typeArgs(typeArgs),
          params(params),
          result(result),
          requirements(requirements),
          body(body)
    {
    }

    virtual bool isConstraintFunction() { return false; }
    virtual bool isIntrinsicFunction() { return false; }
    virtual bool isNormalFunction() { return false; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        llvmFunction, llvm::Function*, getLLVMFunction, setLLVMFunction
    )

    METADATA_PROP(noMangle, bool, getNoMangle, setNoMangle)
    METADATA_PROP(extern_, bool, getExtern, setExtern)
};

struct IRConstraintFunction : public IRFunction
{
    IRConstraintFunction(
        std::string const& name,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result
    )
        : IRFunction(
            nullptr, nullptr, name, {}, {}, params, result, {}, nullptr
        )
    {
    }

    bool isConstraintFunction() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRIntrinsicFunction : public IRFunction
{
    IRIntrinsicFunction() {}

    IRIntrinsicFunction(
        IRModule* parent,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<IRConstraintInstantiation*> requirements
    )
        : IRFunction(
            parent,
            nullptr,
            name,
            typeParams,
            {},
            params,
            result,
            requirements,
            nullptr
        )
    {
    }

    bool isIntrinsicFunction() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRNormalFunction : public IRFunction
{
    IRNormalFunction() {}

    IRNormalFunction(
        IRModule* parent,
        IRNormalFunction* blueprint,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<IRType*> const& typeArgs,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<IRConstraintInstantiation*> requirements,
        IRStatement* body
    )
        : IRFunction(
            parent,
            blueprint,
            name,
            typeParams,
            typeArgs,
            params,
            result,
            requirements,
            body
        )
    {
    }

    bool isNormalFunction() override { return true; }

    IMPLEMENT_ACCEPT()
};