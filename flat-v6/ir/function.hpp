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

    IRFunction() {}

    IRFunction(
        IRModule* parent,
        IRFunction* blueprint,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<IRType*> const& typeArgs,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<IRConstraintInstantiation*> requirements
    )
        : parent(parent),
          blueprint(blueprint),
          name(name),
          typeParams(typeParams),
          typeArgs(typeArgs),
          params(params),
          result(result),
          requirements(requirements)
    {
    }

    virtual bool isConstraintFunction() { return false; }
    virtual bool isIntrinsicFunction() { return false; }
    virtual bool isNormalFunction() { return false; }

    IMPLEMENT_ACCEPT()
};

struct IRConstraintFunction : public IRFunction
{
    IRConstraintFunction(
        std::string const& name,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result
    )
        : IRFunction(nullptr, nullptr, name, {}, {}, params, result, {})
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
        IRIntrinsicFunction* blueprint,
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::vector<IRType*> const& typeArgs,
        std::vector<std::pair<std::string, IRType*>> const& params,
        IRType* result,
        std::vector<IRConstraintInstantiation*> requirements
    )
        : IRFunction(
            parent,
            blueprint,
            name,
            typeParams,
            typeArgs,
            params,
            result,
            requirements
        )
    {
    }

    bool isIntrinsicFunction() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRNormalFunction : public IRFunction
{
    IRStatement* body = nullptr;

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
            requirements
        ),
          body(body)
    {
    }

    bool isNormalFunction() override { return true; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        llvmFunction, llvm::Function*, getLLVMFunction, setLLVMFunction
    )
};