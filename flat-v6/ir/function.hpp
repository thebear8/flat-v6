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
    virtual bool isUnaryIntrinsic() { return false; }
    virtual bool isBinaryIntrinsic() { return false; }
    virtual bool isIndexIntrinsic() { return false; }
    virtual bool isNormalFunction() { return false; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        llvmFunction, llvm::Function*, getLLVMFunction, setLLVMFunction
    )
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

struct IRUnaryIntrinsic : IRFunction
{
    IRUnaryIntrinsic() {}

    IRUnaryIntrinsic(std::string name, IRType* a, IRType* result)
        : IRFunction(
            nullptr, nullptr, name, {}, {}, { { "a", a } }, result, {}, nullptr
        )
    {
    }

    bool isUnaryIntrinsic() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRBinaryIntrinsic : public IRFunction
{
    IRBinaryIntrinsic() {}

    IRBinaryIntrinsic(std::string name, IRType* a, IRType* b, IRType* result)
        : IRFunction(
            nullptr,
            nullptr,
            name,
            {},
            {},
            { { "a", a }, { "b", b } },
            result,
            {},
            nullptr
        )
    {
    }

    bool isBinaryIntrinsic() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRIndexIntrinsic : IRFunction
{
    IRIndexIntrinsic() {}

    IRIndexIntrinsic(IRGenericType* t, IRArrayType* arrayOfT, IRType* idx)
        : IRFunction(
            nullptr,
            nullptr,
            "__index__",
            { t },
            {},
            { { "array", (IRType*)arrayOfT }, { "index", (IRType*)idx } },
            (IRType*)t,
            {},
            nullptr
        )
    {
    }

    bool isIndexIntrinsic() override { return true; }

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